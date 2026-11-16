/* Private v2 row-stream codec. Pinned codecs and identical encode settings.
 * Budget/Block match the frozen owned JPEG system allocator's C-compatible
 * client_data layout, including allocation headers and fail/status accounting.
 * C callbacks return errors; no C++ exception crosses codec longjmp frames. */
#include "stream_codec_v2.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#define JPEG_INTERNALS
#include "jpeglib.h"
#include "png.h"
enum { MALFORMED=17, MISMATCH=19, LIMIT=8 };
typedef struct Budget { size_t limit,live; int fail_after,status; } Budget;
/* Align all scalar codec objects without depending on MSVC's C max_align_t.
 * These portable builds contain no vector/SIMD allocations.
 */
typedef union Block { long double align; void* pointer; uint64_t integer; struct { size_t bytes; } data; } Block;
static void* take(Budget* b,size_t n) {
    Block* p;
    if(n>SIZE_MAX-sizeof(Block)||b->live>b->limit||n+sizeof(Block)>b->limit-b->live||b->fail_after==0) { b->status=LIMIT;return NULL; }
    if(b->fail_after>0)--b->fail_after;
    p=(Block*)malloc(n+sizeof(Block));if(!p){b->status=LIMIT;return NULL;}
    p->data.bytes=n+sizeof(Block);b->live+=p->data.bytes;return p+1;
}
static void drop(Budget* b,void* v) { if(v){Block* p=(Block*)v-1;b->live-=p->data.bytes;free(p);} }

typedef struct Stream {
    Budget budget;
    jmp_buf jump;
    struct jpeg_compress_struct jpeg;
    struct jpeg_error_mgr error;
    struct jpeg_destination_mgr destination;
    png_structp png;png_infop info;
    pixaura_private_row_v2 row;pixaura_private_bytes_v2 write;void* user;
    uint8_t chunk[65536];uint8_t* rgba;uint8_t* rgb;
} Stream;
static void jpeg_error(j_common_ptr c){Stream* s=(Stream*)c->client_data;if(!s->budget.status)s->budget.status=MALFORMED;longjmp(s->jump,1);}
static void jpeg_warning(j_common_ptr c,int level){if(level<0)jpeg_error(c);}
static void emit(Stream* s,const uint8_t* p,size_t n){int32_t code=s->write(s->user,p,n);if(code){s->budget.status=code;longjmp(s->jump,1);}}
static void output_init(j_compress_ptr c){Stream* s=(Stream*)c->client_data;s->destination.next_output_byte=s->chunk;s->destination.free_in_buffer=sizeof(s->chunk);}
static boolean output_full(j_compress_ptr c){Stream* s=(Stream*)c->client_data;emit(s,s->chunk,sizeof(s->chunk));output_init(c);return TRUE;}
static void output_end(j_compress_ptr c){Stream* s=(Stream*)c->client_data;emit(s,s->chunk,sizeof(s->chunk)-s->destination.free_in_buffer);}
static void stream_png_error(png_structp p,png_const_charp text){Stream* s=(Stream*)png_get_error_ptr(p);(void)text;if(!s->budget.status)s->budget.status=MALFORMED;longjmp(s->jump,1);}
static png_voidp png_allocate(png_structp p,png_alloc_size_t n){return take(&((Stream*)png_get_mem_ptr(p))->budget,(size_t)n);}
static void png_release(png_structp p,png_voidp v){drop(&((Stream*)png_get_mem_ptr(p))->budget,v);}
static void png_output(png_structp p,png_bytep data,png_size_t n){emit((Stream*)png_get_io_ptr(p),data,n);}
int32_t pixaura_private_stream_encode_v2(uint32_t format,uint32_t w,uint32_t h,int quality,size_t scratch,
    pixaura_private_row_v2 row,pixaura_private_bytes_v2 write,void* user){
    Stream* s;int32_t status=0;uint32_t y;
    if((format!=1&&format!=2)||!w||!h||w>16384||h>16384||!row||!write||quality<1||quality>100||scratch<sizeof(Stream)+(size_t)w*7)return LIMIT;
    s=(Stream*)calloc(1,sizeof(Stream));if(!s)return LIMIT;
    s->budget.limit=scratch-sizeof(Stream);s->budget.fail_after=-1;s->row=row;s->write=write;s->user=user;
    if(setjmp(s->jump)){status=s->budget.status?s->budget.status:MALFORMED;goto done;}
    s->rgba=(uint8_t*)take(&s->budget,(size_t)w*4);if(!s->rgba){status=LIMIT;goto done;}
    if(format==1){
        s->rgb=(uint8_t*)take(&s->budget,(size_t)w*3);if(!s->rgb){status=LIMIT;goto done;}
        jpeg_std_error(&s->error);s->error.error_exit=jpeg_error;s->error.emit_message=jpeg_warning;
        s->jpeg.err=&s->error;s->jpeg.client_data=s;
        jpeg_create_compress(&s->jpeg);
        s->destination.init_destination=output_init;s->destination.empty_output_buffer=output_full;s->destination.term_destination=output_end;s->jpeg.dest=&s->destination;
        s->jpeg.image_width=w;s->jpeg.image_height=h;s->jpeg.input_components=3;s->jpeg.in_color_space=JCS_RGB;
        jpeg_set_defaults(&s->jpeg);jpeg_set_quality(&s->jpeg,quality,TRUE);s->jpeg.dct_method=JDCT_ISLOW;s->jpeg.optimize_coding=FALSE;
        jpeg_start_compress(&s->jpeg,TRUE);
        for(y=0;y<h;++y){uint32_t x;JSAMPROW scan=s->rgb;status=row(user,y,s->rgba,(size_t)w*4);if(status)goto done;
            for(x=0;x<w;++x){uint32_t c;const uint32_t a=s->rgba[x*4+3];for(c=0;c<3;++c)s->rgb[x*3+c]=(uint8_t)((s->rgba[x*4+c]*a+255u*(255u-a)+127u)/255u);}
            if(jpeg_write_scanlines(&s->jpeg,&scan,1)!=1){status=MALFORMED;goto done;}
        }
        jpeg_finish_compress(&s->jpeg);
    }else{
        s->png=png_create_write_struct_2(PNG_LIBPNG_VER_STRING,s,stream_png_error,stream_png_error,s,png_allocate,png_release);if(!s->png){status=LIMIT;goto done;}
        s->info=png_create_info_struct(s->png);if(!s->info){status=LIMIT;goto done;}
        png_set_write_fn(s->png,s,png_output,NULL);
        png_set_IHDR(s->png,s->info,w,h,8,PNG_COLOR_TYPE_RGB_ALPHA,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_BASE,PNG_FILTER_TYPE_BASE);
        png_set_filter(s->png,0,PNG_FILTER_NONE);png_set_compression_level(s->png,6);png_write_info(s->png,s->info);
        for(y=0;y<h;++y){status=row(user,y,s->rgba,(size_t)w*4);if(status)goto done;png_write_row(s->png,s->rgba);}
        png_write_end(s->png,s->info);
    }
done:
    if(s->jpeg.mem)jpeg_destroy_compress(&s->jpeg);
    if(s->png)png_destroy_write_struct(&s->png,&s->info);
    drop(&s->budget,s->rgba);drop(&s->budget,s->rgb);
    if(s->budget.live&&!status)status=MISMATCH;
    free(s);return status;
}
