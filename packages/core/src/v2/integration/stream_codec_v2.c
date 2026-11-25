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
    jmp_buf* jump;
    struct jpeg_compress_struct jpeg;
    struct jpeg_error_mgr error;
    struct jpeg_destination_mgr destination;
    png_structp png;png_infop info;
    pixaura_private_row_v2 row;pixaura_private_bytes_v2 write;void* user;
    uint8_t chunk[65536];uint8_t* rgba;uint8_t* rgb;
} Stream;
static void jpeg_error(j_common_ptr c){Stream* s=(Stream*)c->client_data;if(!s->budget.status)s->budget.status=MALFORMED;longjmp(*s->jump,1);}
static void jpeg_warning(j_common_ptr c,int level){if(level<0)jpeg_error(c);}
static void emit(Stream* s,const uint8_t* p,size_t n){int32_t code=s->write(s->user,p,n);if(code){s->budget.status=code;longjmp(*s->jump,1);}}
static void output_init(j_compress_ptr c){Stream* s=(Stream*)c->client_data;s->destination.next_output_byte=s->chunk;s->destination.free_in_buffer=sizeof(s->chunk);}
static boolean output_full(j_compress_ptr c){Stream* s=(Stream*)c->client_data;emit(s,s->chunk,sizeof(s->chunk));output_init(c);return TRUE;}
static void output_end(j_compress_ptr c){Stream* s=(Stream*)c->client_data;emit(s,s->chunk,sizeof(s->chunk)-s->destination.free_in_buffer);}
static void stream_png_error(png_structp p,png_const_charp text){Stream* s=(Stream*)png_get_error_ptr(p);(void)text;if(!s->budget.status)s->budget.status=MALFORMED;longjmp(*s->jump,1);}
static png_voidp png_allocate(png_structp p,png_alloc_size_t n){return take(&((Stream*)png_get_mem_ptr(p))->budget,(size_t)n);}
static void png_release(png_structp p,png_voidp v){drop(&((Stream*)png_get_mem_ptr(p))->budget,v);}
static void png_output(png_structp p,png_bytep data,png_size_t n){emit((Stream*)png_get_io_ptr(p),data,n);}
int32_t pixaura_private_stream_encode_v2(uint32_t format,uint32_t w,uint32_t h,int quality,size_t scratch,
    pixaura_private_row_v2 row,pixaura_private_bytes_v2 write,void* user){
    /* MSVC jmp_buf has explicit alignment. Keep it as an aligned C stack
     * object instead of introducing hidden padding in the codec state. Its
     * lifetime covers all callbacks, including cleanup after longjmp. */
    jmp_buf jump;Stream* s;int32_t status=0;uint32_t y;
    if((format!=1&&format!=2)||!w||!h||w>16384||h>16384||!row||!write||quality<(format==2?0:1)||quality>100||scratch<sizeof(Stream)+sizeof(jump)+(size_t)w*7)return LIMIT;
    s=(Stream*)calloc(1,sizeof(Stream));if(!s)return LIMIT;
    s->jump=&jump;s->budget.limit=scratch-sizeof(Stream)-sizeof(jump);s->budget.fail_after=-1;s->row=row;s->write=write;s->user=user;
    if(setjmp(jump)){status=s->budget.status?s->budget.status:MALFORMED;goto done;}
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

/* Additive managed-source decoder. Byte equivalence with decode_codec.c is a
 * test obligation; the frozen decoder and pinned libraries remain unchanged. */
typedef struct ReadRows {
    Budget budget;jmp_buf* jump;
    struct jpeg_decompress_struct jpeg;struct jpeg_error_mgr error;
    struct jpeg_source_mgr source;
    png_structp png;png_infop info;
    const uint8_t* input;size_t bytes,offset;
    pixaura_private_raster_row_v2 exchange;void* user;uint8_t* rgba;
} ReadRows;
static void read_jpeg_error(j_common_ptr c){ReadRows* s=(ReadRows*)c->client_data;if(!s->budget.status)s->budget.status=MALFORMED;longjmp(*s->jump,1);}
static void read_jpeg_warning(j_common_ptr c,int level){if(level<0)read_jpeg_error(c);}
static void read_source_noop(j_decompress_ptr c){(void)c;}
static boolean read_source_eof(j_decompress_ptr c){((ReadRows*)c->client_data)->budget.status=18;read_jpeg_error((j_common_ptr)c);return FALSE;}
static void read_source_skip(j_decompress_ptr c,long count){
    if(count<=0)return;
    if((size_t)count>c->src->bytes_in_buffer){(void)read_source_eof(c);return;}
    c->src->next_input_byte+=(size_t)count;c->src->bytes_in_buffer-=(size_t)count;
}
static void read_png_error(png_structp p,png_const_charp message){ReadRows* s=(ReadRows*)png_get_error_ptr(p);(void)message;if(!s->budget.status)s->budget.status=MALFORMED;longjmp(*s->jump,1);}
static png_voidp read_png_allocate(png_structp p,png_alloc_size_t n){return take(&((ReadRows*)png_get_mem_ptr(p))->budget,(size_t)n);}
static void read_png_release(png_structp p,png_voidp v){drop(&((ReadRows*)png_get_mem_ptr(p))->budget,v);}
static void read_png_bytes(png_structp p,png_bytep output,png_size_t n){ReadRows* s=(ReadRows*)png_get_io_ptr(p);if(n>s->bytes-s->offset){s->budget.status=18;read_png_error(p,"EOF");return;}memcpy(output,s->input+s->offset,n);s->offset+=n;}
int32_t pixaura_private_stream_decode_v2(uint32_t format,const uint8_t* input,size_t bytes,uint32_t w,uint32_t h,size_t scratch,
    pixaura_private_raster_row_v2 exchange,void* user){
    jmp_buf jump;ReadRows* s;int32_t status=0;uint32_t y;int color,depth,passes,pass;
    if((format!=1&&format!=2)||!input||!bytes||!w||!h||w>16384||h>16384||!exchange||scratch>(32u<<20)||scratch<sizeof(ReadRows)+sizeof(jump)+(size_t)w*4)return LIMIT;
    s=(ReadRows*)calloc(1,sizeof(ReadRows));if(!s)return LIMIT;
    s->jump=&jump;s->budget.limit=scratch-sizeof(ReadRows)-sizeof(jump);s->budget.fail_after=-1;
    s->input=input;s->bytes=bytes;s->exchange=exchange;s->user=user;
    if(setjmp(jump)){status=s->budget.status?s->budget.status:MALFORMED;goto read_done;}
    s->rgba=(uint8_t*)take(&s->budget,(size_t)w*4);if(!s->rgba){status=LIMIT;goto read_done;}
    if(format==1){
        jpeg_std_error(&s->error);s->error.error_exit=read_jpeg_error;s->error.emit_message=read_jpeg_warning;
        s->jpeg.err=&s->error;s->jpeg.client_data=s;jpeg_create_decompress(&s->jpeg);
        s->source.init_source=read_source_noop;s->source.fill_input_buffer=read_source_eof;s->source.skip_input_data=read_source_skip;
        s->source.resync_to_restart=jpeg_resync_to_restart;s->source.term_source=read_source_noop;s->source.next_input_byte=input;s->source.bytes_in_buffer=bytes;s->jpeg.src=&s->source;
        if(jpeg_read_header(&s->jpeg,TRUE)!=JPEG_HEADER_OK){status=MALFORMED;goto read_done;}
        if(s->jpeg.image_width!=w||s->jpeg.image_height!=h||s->jpeg.data_precision!=8||s->jpeg.progressive_mode||s->jpeg.arith_code){status=MISMATCH;goto read_done;}
        s->jpeg.out_color_space=JCS_EXT_RGBA;s->jpeg.dct_method=JDCT_ISLOW;s->jpeg.do_fancy_upsampling=FALSE;
        if(!jpeg_start_decompress(&s->jpeg)||s->jpeg.output_width!=w||s->jpeg.output_height!=h||s->jpeg.output_components!=4){status=MISMATCH;goto read_done;}
        while(s->jpeg.output_scanline<h){JSAMPROW row=s->rgba;y=s->jpeg.output_scanline;
            if(jpeg_read_scanlines(&s->jpeg,&row,1)!=1){status=18;goto read_done;}
            status=exchange(user,y,s->rgba,(size_t)w*4,1);if(status)goto read_done;
        }
        if(!jpeg_finish_decompress(&s->jpeg)||s->source.bytes_in_buffer!=0)status=MALFORMED;
    }else{
        s->png=png_create_read_struct_2(PNG_LIBPNG_VER_STRING,s,read_png_error,read_png_error,s,read_png_allocate,read_png_release);if(!s->png){status=LIMIT;goto read_done;}
        s->info=png_create_info_struct(s->png);if(!s->info){status=LIMIT;goto read_done;}
        png_set_read_fn(s->png,s,read_png_bytes);png_set_user_limits(s->png,w,h);png_set_crc_action(s->png,PNG_CRC_ERROR_QUIT,PNG_CRC_ERROR_QUIT);
        png_set_keep_unknown_chunks(s->png,PNG_HANDLE_CHUNK_NEVER,NULL,0);png_read_info(s->png,s->info);
        color=png_get_color_type(s->png,s->info);depth=png_get_bit_depth(s->png,s->info);
        if(png_get_image_width(s->png,s->info)!=w||png_get_image_height(s->png,s->info)!=h||depth>8){status=MISMATCH;goto read_done;}
        if(color==PNG_COLOR_TYPE_PALETTE)png_set_palette_to_rgb(s->png);
        if(color==PNG_COLOR_TYPE_GRAY&&depth<8)png_set_expand_gray_1_2_4_to_8(s->png);
        if(png_get_valid(s->png,s->info,PNG_INFO_tRNS))png_set_tRNS_to_alpha(s->png);
        if(color==PNG_COLOR_TYPE_GRAY||color==PNG_COLOR_TYPE_GRAY_ALPHA)png_set_gray_to_rgb(s->png);
        if(!(color&PNG_COLOR_MASK_ALPHA)&&!png_get_valid(s->png,s->info,PNG_INFO_tRNS))png_set_add_alpha(s->png,255,PNG_FILLER_AFTER);
        passes=png_set_interlace_handling(s->png);png_read_update_info(s->png,s->info);
        if(png_get_rowbytes(s->png,s->info)!=(size_t)w*4||png_get_channels(s->png,s->info)!=4){status=MISMATCH;goto read_done;}
        for(pass=0;pass<passes;++pass)for(y=0;y<h;++y){
            if(pass){status=exchange(user,y,s->rgba,(size_t)w*4,0);if(status)goto read_done;}
            else memset(s->rgba,0,(size_t)w*4);
            png_read_row(s->png,s->rgba,NULL);status=exchange(user,y,s->rgba,(size_t)w*4,1);if(status)goto read_done;
        }
        png_read_end(s->png,NULL);if(s->offset!=bytes)status=MALFORMED;
    }
read_done:
    if(s->jpeg.mem)jpeg_destroy_decompress(&s->jpeg);
    if(s->png)png_destroy_read_struct(&s->png,&s->info,NULL);
    drop(&s->budget,s->rgba);if(s->budget.live&&!status)status=MISMATCH;
    free(s);return status;
}
