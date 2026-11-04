/* Owned adapter, including libjpeg's documented system-memory backend.
 * No vendor source changes. C isolates codec longjmp from C++ destructors.
 */
#include "decode_codec.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>
#define JPEG_INTERNALS
#include "jpeglib.h"
#include "jmemsys.h"
#include "png.h"
#include "zlib.h"
enum { MALFORMED=17, TRUNCATED=18, MISMATCH=19, LIMIT=8 };
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
void* jpeg_get_small(j_common_ptr c,size_t n){return take((Budget*)c->client_data,n);}
void* jpeg_get_large(j_common_ptr c,size_t n){return take((Budget*)c->client_data,n);}
void jpeg_free_small(j_common_ptr c,void* p,size_t n){(void)n;drop((Budget*)c->client_data,p);}
void jpeg_free_large(j_common_ptr c,void* p,size_t n){(void)n;drop((Budget*)c->client_data,p);}
size_t jpeg_mem_available(j_common_ptr c,size_t min,size_t max,size_t used){Budget* b=(Budget*)c->client_data;(void)min;(void)max;(void)used;return b->limit-b->live;}
void jpeg_open_backing_store(j_common_ptr c,backing_store_ptr s,long n){(void)s;(void)n;((Budget*)c->client_data)->status=LIMIT;(*c->err->error_exit)(c);}
long jpeg_mem_init(j_common_ptr c){(void)c;return 0;}
void jpeg_mem_term(j_common_ptr c){(void)c;}
typedef struct Jpeg {
    Budget budget;struct jpeg_decompress_struct codec;struct jpeg_error_mgr error;
    struct jpeg_source_mgr source;jmp_buf jump;
} Jpeg;
/* error manager need not be first; retrieve owning state through budget. */
typedef struct JpegError { struct jpeg_error_mgr base; jmp_buf* jump; Budget* budget; } JpegError;
static void jpeg_fail(j_common_ptr c){JpegError* e=(JpegError*)c->err;if(!e->budget->status)e->budget->status=MALFORMED;longjmp(*e->jump,1);}
static void jpeg_warning(j_common_ptr c,int level){if(level<0)jpeg_fail(c);}
static void source_noop(j_decompress_ptr c){(void)c;}
static boolean source_eof(j_decompress_ptr c){((Budget*)c->client_data)->status=TRUNCATED;jpeg_fail((j_common_ptr)c);return FALSE;}
static void source_skip(j_decompress_ptr c,long n){if(n>0){if((size_t)n>c->src->bytes_in_buffer){source_eof(c);return;}c->src->next_input_byte+=(size_t)n;c->src->bytes_in_buffer-=(size_t)n;}}
static int32_t decode_jpeg(const uint8_t* input,size_t bytes,uint8_t* out,uint32_t w,uint32_t h,size_t stride,size_t cap,int fail_after,int fault){
    Jpeg* s;JpegError e;int32_t status=0;
    if(cap<sizeof(Jpeg))return LIMIT;
    s=(Jpeg*)calloc(1,sizeof(Jpeg));
    if(!s)return LIMIT;
    s->budget.limit=cap-sizeof(Jpeg);s->budget.fail_after=fail_after;
    memset(&e,0,sizeof(e));jpeg_std_error(&e.base);e.base.error_exit=jpeg_fail;e.base.emit_message=jpeg_warning;e.jump=&s->jump;e.budget=&s->budget;
    s->codec.err=&e.base;s->codec.client_data=&s->budget;
    if(setjmp(s->jump)){status=s->budget.status;goto done;}
    if(fault==1){status=MALFORMED;goto done;}
    jpeg_create_decompress(&s->codec);
    s->source.init_source=source_noop;s->source.fill_input_buffer=source_eof;s->source.skip_input_data=source_skip;s->source.resync_to_restart=jpeg_resync_to_restart;s->source.term_source=source_noop;s->source.next_input_byte=input;s->source.bytes_in_buffer=bytes;s->codec.src=&s->source;
    if(jpeg_read_header(&s->codec,TRUE)!=JPEG_HEADER_OK){status=MALFORMED;goto done;}
    if(s->codec.image_width!=w||s->codec.image_height!=h||s->codec.data_precision!=8||s->codec.progressive_mode||s->codec.arith_code){status=MISMATCH;goto done;}
    s->codec.out_color_space=JCS_EXT_RGBA;s->codec.dct_method=JDCT_ISLOW;s->codec.do_fancy_upsampling=FALSE;
    if(!jpeg_start_decompress(&s->codec)||s->codec.output_width!=w||s->codec.output_height!=h||s->codec.output_components!=4||fault==2){status=MISMATCH;goto done;}
    while(s->codec.output_scanline<h){JSAMPROW row=out+(size_t)s->codec.output_scanline*stride;if(jpeg_read_scanlines(&s->codec,&row,1)!=1){status=TRUNCATED;goto done;}}
    if(!jpeg_finish_decompress(&s->codec)||s->source.bytes_in_buffer!=0)status=MALFORMED;
done:
    jpeg_destroy_decompress(&s->codec);if(s->budget.live!=0)status=MISMATCH;free(s);return status;
}
typedef struct Png { Budget budget;const uint8_t* input;size_t bytes,offset;png_structp codec;png_infop info;jmp_buf jump; } Png;
static void png_fail(png_structp p,png_const_charp msg){Png* s=(Png*)png_get_error_ptr(p);(void)msg;if(!s->budget.status)s->budget.status=MALFORMED;longjmp(s->jump,1);}
static void png_warn(png_structp p,png_const_charp msg){png_fail(p,msg);}
static png_voidp png_take(png_structp p,png_alloc_size_t n){return take(&((Png*)png_get_mem_ptr(p))->budget,(size_t)n);}
static void png_drop(png_structp p,png_voidp v){drop(&((Png*)png_get_mem_ptr(p))->budget,v);}
static void png_read(png_structp p,png_bytep v,png_size_t n){Png* s=(Png*)png_get_io_ptr(p);if(n>s->bytes-s->offset){s->budget.status=TRUNCATED;png_fail(p,"EOF");return;}memcpy(v,s->input+s->offset,n);s->offset+=n;}
static int32_t decode_png(const uint8_t* input,size_t bytes,uint8_t* out,uint32_t w,uint32_t h,size_t stride,size_t cap,int fail_after,int fault){
    Png* s;int32_t status=0;int color,depth,passes,pass;uint32_t y;
    if(cap<sizeof(Png))return LIMIT;
    s=(Png*)calloc(1,sizeof(Png));
    if(!s)return LIMIT;
    s->budget.limit=cap-sizeof(Png);s->budget.fail_after=fail_after;s->input=input;s->bytes=bytes;
    if(setjmp(s->jump)){status=s->budget.status;goto done;}
    if(fault==1){status=MALFORMED;goto done;}
    s->codec=png_create_read_struct_2(PNG_LIBPNG_VER_STRING,s,png_fail,png_warn,s,png_take,png_drop);if(!s->codec){status=LIMIT;goto done;}
    s->info=png_create_info_struct(s->codec);if(!s->info){status=LIMIT;goto done;}
    png_set_read_fn(s->codec,s,png_read);png_set_user_limits(s->codec,w,h);
    png_set_crc_action(s->codec,PNG_CRC_ERROR_QUIT,PNG_CRC_ERROR_QUIT);
    /* Metadata is independently validated/retained; never decompress text here. */
    png_set_keep_unknown_chunks(s->codec,PNG_HANDLE_CHUNK_NEVER,NULL,0);
    png_read_info(s->codec,s->info);
    color=png_get_color_type(s->codec,s->info);depth=png_get_bit_depth(s->codec,s->info);
    if(png_get_image_width(s->codec,s->info)!=w||png_get_image_height(s->codec,s->info)!=h||depth>8){status=MISMATCH;goto done;}
    if(color==PNG_COLOR_TYPE_PALETTE)png_set_palette_to_rgb(s->codec);
    if(color==PNG_COLOR_TYPE_GRAY&&depth<8)png_set_expand_gray_1_2_4_to_8(s->codec);
    if(png_get_valid(s->codec,s->info,PNG_INFO_tRNS))png_set_tRNS_to_alpha(s->codec);
    if(color==PNG_COLOR_TYPE_GRAY||color==PNG_COLOR_TYPE_GRAY_ALPHA)png_set_gray_to_rgb(s->codec);
    if(!(color&PNG_COLOR_MASK_ALPHA)&&!png_get_valid(s->codec,s->info,PNG_INFO_tRNS))png_set_add_alpha(s->codec,255,PNG_FILLER_AFTER);
    passes=png_set_interlace_handling(s->codec);png_read_update_info(s->codec,s->info);
    if(png_get_rowbytes(s->codec,s->info)!=stride||png_get_channels(s->codec,s->info)!=4||fault==2){status=MISMATCH;goto done;}
    for(pass=0;pass<passes;++pass)for(y=0;y<h;++y)png_read_row(s->codec,out+(size_t)y*stride,NULL);
    png_read_end(s->codec,NULL);if(s->offset!=bytes)status=MALFORMED;
done:
    png_destroy_read_struct(&s->codec,&s->info,NULL);if(s->budget.live!=0)status=MISMATCH;free(s);return status;
}
int32_t pixaura_codec_decode(uint32_t f,const uint8_t* in,size_t n,uint8_t* out,uint32_t w,uint32_t h,size_t stride,size_t cap,int fail_after,int fault){return f==1?decode_jpeg(in,n,out,w,h,stride,cap,fail_after,fault):decode_png(in,n,out,w,h,stride,cap,fail_after,fault);}
static voidpf ztake(voidpf p,uInt n,uInt size){if(size&&n>SIZE_MAX/size)return NULL;return take((Budget*)p,(size_t)n*size);}
static void zdrop(voidpf p,voidpf v){drop((Budget*)p,v);}
int32_t pixaura_codec_inflate(const uint8_t* input,size_t n,uint8_t* out,size_t cap,size_t* actual,size_t scratch){
    z_stream z;Budget b;int status;memset(&z,0,sizeof(z));memset(&b,0,sizeof(b));b.limit=scratch;b.fail_after=-1;z.zalloc=ztake;z.zfree=zdrop;z.opaque=&b;
    if(n>UINT32_MAX||cap>UINT32_MAX)return LIMIT;
    if(inflateInit(&z)!=Z_OK)return b.status?b.status:MALFORMED;
    z.next_in=(Bytef*)input;z.avail_in=(uInt)n;z.next_out=out;z.avail_out=(uInt)cap;
    status=inflate(&z,Z_FINISH);
    if(status==Z_STREAM_END&&z.avail_in==0){*actual=(size_t)z.total_out;status=0;}
    else status=b.status?b.status:(z.avail_out==0?LIMIT:MALFORMED);
    inflateEnd(&z);return status;
}
typedef struct JpegOut { struct jpeg_destination_mgr pub; uint8_t* out; size_t cap,used; Budget* budget; } JpegOut;
typedef struct JpegEncode { Budget budget; struct jpeg_compress_struct codec; jmp_buf jump; } JpegEncode;
static void jpeg_out_init(j_compress_ptr c){JpegOut* d=(JpegOut*)c->dest;d->pub.next_output_byte=d->out;d->pub.free_in_buffer=d->cap;}
static boolean jpeg_out_empty(j_compress_ptr c){((JpegOut*)c->dest)->budget->status=LIMIT;jpeg_fail((j_common_ptr)c);return FALSE;}
static void jpeg_out_term(j_compress_ptr c){JpegOut* d=(JpegOut*)c->dest;d->used=d->cap-d->pub.free_in_buffer;}
int32_t pixaura_codec_encode_jpeg(const uint8_t* rgba,uint32_t w,uint32_t h,int quality,uint8_t* out,size_t cap,size_t* written,size_t scratch){
    JpegEncode* s;JpegError e;JpegOut dest;uint8_t* row;int32_t status=0;uint32_t y;
    if(!rgba||!out||!written||!w||!h||quality<1||quality>100||cap<64)return LIMIT;
    if(scratch<sizeof(JpegEncode)+((size_t)w*3))return LIMIT;
    s=(JpegEncode*)calloc(1,sizeof(JpegEncode));if(!s)return LIMIT;
    s->budget.limit=scratch-sizeof(JpegEncode);s->budget.fail_after=-1;
    row=(uint8_t*)take(&s->budget,(size_t)w*3);if(!row){free(s);return LIMIT;}
    memset(&e,0,sizeof(e));jpeg_std_error(&e.base);e.base.error_exit=jpeg_fail;e.base.emit_message=jpeg_warning;e.jump=&s->jump;e.budget=&s->budget;
    s->codec.err=&e.base;s->codec.client_data=&s->budget;
    if(setjmp(s->jump)){status=s->budget.status?s->budget.status:MALFORMED;goto done;}
    jpeg_create_compress(&s->codec);
    memset(&dest,0,sizeof(dest));dest.pub.init_destination=jpeg_out_init;dest.pub.empty_output_buffer=jpeg_out_empty;dest.pub.term_destination=jpeg_out_term;dest.out=out;dest.cap=cap;dest.budget=&s->budget;s->codec.dest=&dest.pub;
    s->codec.image_width=w;s->codec.image_height=h;s->codec.input_components=3;s->codec.in_color_space=JCS_RGB;
    jpeg_set_defaults(&s->codec);jpeg_set_quality(&s->codec,quality,TRUE);s->codec.dct_method=JDCT_ISLOW;s->codec.optimize_coding=FALSE;
    jpeg_start_compress(&s->codec,TRUE);
    for(y=0;y<h;++y){const uint8_t* src=rgba+(size_t)y*w*4;uint32_t x;JSAMPROW scan=row;
        for(x=0;x<w;++x){const uint32_t a=src[x*4+3];uint32_t c;for(c=0;c<3;++c)row[x*3+c]=(uint8_t)((src[x*4+c]*a+255u*(255u-a)+127u)/255u);}
        if(jpeg_write_scanlines(&s->codec,&scan,1)!=1){status=MALFORMED;goto done;}}
    jpeg_finish_compress(&s->codec);*written=dest.used;if(!*written)status=MALFORMED;
done:
    jpeg_destroy_compress(&s->codec);drop(&s->budget,row);if(s->budget.live!=0&&!status)status=MISMATCH;free(s);return status;
}
typedef struct EncodePng { Budget budget;uint8_t* out;size_t cap,used;png_structp codec;png_infop info;jmp_buf jump; } EncodePng;
static void epng_fail(png_structp p,png_const_charp msg){EncodePng* s=(EncodePng*)png_get_error_ptr(p);(void)msg;if(!s->budget.status)s->budget.status=MALFORMED;longjmp(s->jump,1);}
static png_voidp epng_take(png_structp p,png_alloc_size_t n){return take(&((EncodePng*)png_get_mem_ptr(p))->budget,(size_t)n);}
static void epng_drop(png_structp p,png_voidp v){drop(&((EncodePng*)png_get_mem_ptr(p))->budget,v);}
static void epng_write(png_structp p,png_bytep data,png_size_t n){EncodePng* s=(EncodePng*)png_get_io_ptr(p);if(n>s->cap-s->used){s->budget.status=LIMIT;epng_fail(p,"cap");return;}memcpy(s->out+s->used,data,n);s->used+=n;}
int32_t pixaura_codec_encode_png(const uint8_t* rgba,uint32_t w,uint32_t h,uint8_t* out,size_t cap,size_t* written,size_t scratch){
    EncodePng* s;int32_t status=0;uint32_t y;
    if(!rgba||!out||!written||!w||!h||cap<64)return LIMIT;
    if(scratch<sizeof(EncodePng))return LIMIT;
    s=(EncodePng*)calloc(1,sizeof(EncodePng));if(!s)return LIMIT;
    s->budget.limit=scratch-sizeof(EncodePng);s->budget.fail_after=-1;s->out=out;s->cap=cap;
    if(setjmp(s->jump)){status=s->budget.status?s->budget.status:MALFORMED;goto done;}
    s->codec=png_create_write_struct_2(PNG_LIBPNG_VER_STRING,s,epng_fail,epng_fail,s,epng_take,epng_drop);if(!s->codec){status=LIMIT;goto done;}
    s->info=png_create_info_struct(s->codec);if(!s->info){status=LIMIT;goto done;}
    png_set_write_fn(s->codec,s,epng_write,NULL);
    png_set_IHDR(s->codec,s->info,w,h,8,PNG_COLOR_TYPE_RGB_ALPHA,PNG_INTERLACE_NONE,PNG_COMPRESSION_TYPE_BASE,PNG_FILTER_TYPE_BASE);
    png_set_filter(s->codec,0,PNG_FILTER_NONE);png_set_compression_level(s->codec,6);
    png_write_info(s->codec,s->info);
    for(y=0;y<h;++y){png_bytep row=(png_bytep)(rgba+(size_t)y*w*4);png_write_row(s->codec,row);}
    png_write_end(s->codec,s->info);*written=s->used;if(!*written)status=MALFORMED;
done:
    png_destroy_write_struct(&s->codec,&s->info);if(s->budget.live!=0&&!status)status=MISMATCH;free(s);return status;
}
