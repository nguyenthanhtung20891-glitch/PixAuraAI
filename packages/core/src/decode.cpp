#include "decode.hpp"
#include "decode_codec.h"
#include "sha256.hpp"
#include <array>
#include <cstring>
#include <limits>
#include <zlib.h>
namespace pixaura::decode {
namespace {
void need(bool ok,int32_t status=17){if(!ok)throw document::Failure{status};}
struct View {
    const uint8_t* p;std::size_t n;
    uint8_t at(std::size_t i)const{need(i<n,18);return p[i];}
    View sub(std::size_t i,std::size_t count)const{need(i<=n&&count<=n-i,18);return {p+i,count};}
    uint16_t be16(std::size_t i)const{auto v=sub(i,2);return uint16_t(uint16_t(v.p[0])<<8|v.p[1]);}
    uint32_t be32(std::size_t i)const{auto v=sub(i,4);return uint32_t(v.p[0])<<24|uint32_t(v.p[1])<<16|uint32_t(v.p[2])<<8|v.p[3];}
    bool is(std::size_t i,const char* text,std::size_t count)const{return i<=n&&count<=n-i&&std::memcmp(p+i,text,count)==0;}
};
uint32_t exif(View v){
    need(v.n>=8);const bool little=v.is(0,"II",2);need(little||v.is(0,"MM",2));
    auto u16=[&](std::size_t i){const auto s=v.sub(i,2);return little?uint16_t(s.p[0]|uint16_t(s.p[1])<<8):s.be16(0);};
    auto u32=[&](std::size_t i){const auto s=v.sub(i,4);return little?uint32_t(s.p[0])|uint32_t(s.p[1])<<8|uint32_t(s.p[2])<<16|uint32_t(s.p[3])<<24:s.be32(0);};
    need(u16(2)==42);uint32_t offset=u32(4),orientation=1;bool found=false;
    std::array<uint32_t,4> visited{};unsigned depth=0;
    while(offset){need(depth<visited.size(),8);for(unsigned j=0;j<depth;++j)need(visited[j]!=offset);visited[depth++]=offset;
        need(offset>=8);const uint16_t count=u16(offset);need(count<=256,8);v.sub(offset+2,std::size_t(count)*12+4);
        for(unsigned j=0;j<count;++j){const auto i=std::size_t(offset)+2+j*12;
            if(u16(i)==0x112&&depth==1){need(!found&&u16(i+2)==3&&u32(i+4)==1);orientation=u16(i+8);need(orientation>=1&&orientation<=8);found=true;}
        }
        offset=u32(std::size_t(offset)+2+std::size_t(count)*12);
    }
    return orientation;
}
void profile(Metadata& m,const pixaura_decode_limits& limits,bool grayscale){
    View v{m.profile.data(),m.profile.size()};need(v.n<=limits.profile_bytes,8);need(v.n>=132&&v.be32(0)==v.n&&v.is(36,"acsp",4));
    need(v.is(16,"RGB ",4)||v.is(16,"GRAY",4),16);need(v.is(20,"XYZ ",4)||v.is(20,"Lab ",4),16);
    need(grayscale?v.is(16,"GRAY",4):v.is(16,"RGB ",4),16);
    need(v.at(8)==2||v.at(8)==4,16);
    const auto count=v.be32(128);need(count<=1024,8);v.sub(132,std::size_t(count)*12);
    for(uint32_t i=0;i<count;++i){const auto pos=132+std::size_t(i)*12;const auto offset=v.be32(pos+4),bytes=v.be32(pos+8);need(offset>=132+count*12&&offset<=v.n&&bytes<=v.n-offset&&bytes>=8);for(uint32_t j=0;j<i;++j)need(v.be32(132+std::size_t(j)*12)!=v.be32(pos));}
    storage::Sha256 hash;hash.update(v.p,v.n);const auto digest=hash.finish();std::memcpy(m.value.profile_sha256,digest.data(),64);m.value.profile_bytes=v.n;m.value.profile_type=1;
}
Metadata png(View v,const pixaura_decode_limits& l){
    Metadata m;bool ihdr=false,idat=false,ended=false,after_data=false,plte=false,trns=false,orientation=false,icc=false,srgb=false,gamma=false,chrm=false;uint64_t metadata=0;uint32_t chunks=0;
    std::size_t pos=8;unsigned color=0,palette_entries=0;
    while(pos<v.n){need(++chunks<=l.marker_count,8);const auto n=v.be32(pos);const auto type=v.sub(pos+4,4);const auto data=v.sub(pos+8,n);v.sub(pos+8+std::size_t(n),4);
        need(n<=0x7fffffff);for(unsigned i=0;i<4;++i)need((type.p[i]>='A'&&type.p[i]<='Z')||(type.p[i]>='a'&&type.p[i]<='z'));need((type.p[2]&32)==0);
        uLong crc=crc32(0,type.p,4);crc=crc32(crc,data.p,n);need(uint32_t(crc)==v.be32(pos+8+std::size_t(n)));
        const auto is=[&](const char* s){return type.is(0,s,4);};
        if(!is("IDAT")){need(n<=l.metadata_bytes-metadata,8);metadata+=n;}
        need(ihdr||is("IHDR"));
        if(is("IHDR")){need(!ihdr&&pos==8&&n==13);ihdr=true;m.value.width=data.be32(0);m.value.height=data.be32(4);m.value.bit_depth=data.at(8);color=data.at(9);
            need(color==0||color==2||color==3||color==4||color==6,16);
            const auto d=m.value.bit_depth;need(d==8||((color==0||color==3)&&(d==1||d==2||d==4)),16);
            need(data.at(10)==0&&data.at(11)==0&&data.at(12)<=1,16);m.value.channels=color==2?3:color==4?2:color==6?4:1;m.value.has_alpha=(color==4||color==6);m.value.orientation=1;m.value.format=2;layout(m.value,l);
        }else if(is("acTL")||is("fcTL")||is("fdAT")||is("cICP")||is("cLLI")||is("mDCV"))need(false,16);
        else if(is("PLTE")){need(!plte&&!idat&&n>0&&n<=768&&n%3==0&&color!=0&&color!=4);if(color==3)need(n/3<=uint32_t(1u<<m.value.bit_depth));plte=true;palette_entries=n/3;}
        else if(is("tRNS")){need(!trns&&!idat&&!m.value.has_alpha);need((color==0&&n==2)||(color==2&&n==6)||(color==3&&plte&&n>0&&n<=palette_entries));if(color==0)need(data.be16(0)<(1u<<m.value.bit_depth));if(color==2)need(data.be16(0)<=255&&data.be16(2)<=255&&data.be16(4)<=255);trns=true;m.value.has_alpha=1;}
        else if(is("IDAT")){need(!after_data&&!ended&&(color!=3||plte));idat=true;}
        else if(is("IEND")){need(!ended&&idat&&n==0);ended=true;need(pos+12==v.n);}
        else if(is("eXIf")){need(!orientation&&n<=l.exif_bytes,n>l.exif_bytes?8:17);orientation=true;m.value.orientation=exif(data);}
        else if(is("iCCP")){need(!icc&&!srgb&&!idat);icc=true;std::size_t z=0;while(z<data.n&&data.p[z])++z;need(z>=1&&z<=79&&z+2<data.n&&data.p[z+1]==0);
            m.profile.resize(static_cast<std::size_t>(l.profile_bytes));std::size_t actual=0;const auto status=pixaura_codec_inflate(data.p+z+2,data.n-z-2,m.profile.data(),m.profile.size(),&actual,static_cast<std::size_t>(l.scratch_bytes));need(status==0,status);m.profile.resize(actual);profile(m,l,color==0||color==4);
        }else if(is("sRGB")){need(!srgb&&!icc&&!idat&&n==1&&data.at(0)<=3);srgb=true;m.value.profile_type=2;}
        else if(is("gAMA")){need(!gamma&&!idat&&n==4&&data.be32(0)>0);gamma=true;}
        else if(is("cHRM")){need(!chrm&&!idat&&n==32);chrm=true;}
        else need((type.p[0]&32)!=0,16);
        if(idat&&!is("IDAT"))after_data=true;
        pos+=12+std::size_t(n);
    }
    need(ended,18);layout(m.value,l);return m;
}
Metadata jpeg(View v,const pixaura_decode_limits& l){
    Metadata m;m.value.orientation=1;bool sof=false,sos=false,done=false,orientation=false;uint64_t meta=0;uint32_t markers=0;std::size_t pos=2;unsigned profiles=0;std::array<View,256> parts{};
    while(pos<v.n){need(v.at(pos++)==0xff);while(pos<v.n&&v.p[pos]==0xff)++pos;const auto marker=v.at(pos++);need(++markers<=l.marker_count,8);
        if(marker==0xd9){need(sos&&pos==v.n);done=true;break;}
        need(marker!=0&&marker!=0xd8&&!(marker>=0xd0&&marker<=0xd7)&&marker!=1);
        const auto len=v.be16(pos);need(len>=2);const auto d=v.sub(pos+2,len-2);pos+=len;
        if(marker>=0xe0||marker==0xfe){need(d.n<=l.metadata_bytes-meta,8);meta+=d.n;}
        if(marker>=0xc0&&marker<=0xcf&&marker!=0xc4&&marker!=0xc8&&marker!=0xcc){need(marker==0xc0,16);need(!sof&&d.n>=6);sof=true;
            m.value.bit_depth=d.at(0);m.value.height=d.be16(1);m.value.width=d.be16(3);m.value.channels=d.at(5);need(m.value.bit_depth==8&&(m.value.channels==1||m.value.channels==3),16);need(d.n==6+m.value.channels*3);
            m.value.format=1;layout(m.value,l);
        }else if(marker==0xe1&&d.is(0,"Exif",4)){need(d.is(0,"Exif\0\0",6)&&!orientation);need(d.n<=l.exif_bytes,8);orientation=true;m.value.orientation=exif(d.sub(6,d.n-6));}
        else if(marker==0xe2&&d.is(0,"ICC_PROFILE\0",12)){need(d.n>=14);const auto seq=d.at(12),count=d.at(13);need(count>0&&seq>0&&seq<=count&&(!profiles||profiles==count)&&parts[seq].p==nullptr);profiles=count;parts[seq]=d.sub(14,d.n-14);}
        else if(marker==0xe2&&d.is(0,"MPF\0",4))need(false,16);
        else if(marker==0xee&&d.is(0,"Adobe",5)){need(d.n==12&&d.at(11)<=1,16);}
        else if(marker==0xda){need(sof&&!sos&&d.n>=4);sos=true;need(d.at(0)==m.value.channels&&d.n==1+m.value.channels*2+3&&d.at(d.n-3)==0&&d.at(d.n-2)==63&&d.at(d.n-1)==0,16);
            // One baseline interleaved scan only. Bound entropy scanning by encoded cap.
            bool next=false;while(pos<v.n){if(v.p[pos++]!=0xff)continue;const auto start=pos-1;while(pos<v.n&&v.p[pos]==0xff)++pos;const auto code=v.at(pos++);if(code==0)continue;if(code>=0xd0&&code<=0xd7){need(++markers<=l.marker_count,8);continue;}pos=start;next=true;break;}need(next,18);
        }
    }
    need(done,18);if(profiles){std::size_t total=0;for(unsigned i=1;i<=profiles;++i){need(parts[i].p!=nullptr);need(parts[i].n<=l.profile_bytes-total,8);total+=parts[i].n;}
        m.profile.reserve(total);for(unsigned i=1;i<=profiles;++i)m.profile.insert(m.profile.end(),parts[i].p,parts[i].p+parts[i].n);profile(m,l,m.value.channels==1);}
    layout(m.value,l);return m;
}
}
pixaura_decode_limits defaults(){return {1,sizeof(pixaura_decode_limits),64ULL<<20,33554432,128ULL<<20,65536,1ULL<<20,512ULL<<10,256ULL<<10,32ULL<<20,256ULL<<20,16384,16384,4096,1};}
void validate_limits(const pixaura_decode_limits& l){const auto d=defaults();need(l.api_version==1&&l.struct_size==sizeof(l),1);
    need(l.encoded_bytes>0&&l.encoded_bytes<=d.encoded_bytes&&l.pixel_count>0&&l.pixel_count<=d.pixel_count&&l.decoded_bytes>0&&l.decoded_bytes<=d.decoded_bytes&&l.row_stride>0&&l.row_stride<=d.row_stride,1);
    need(l.metadata_bytes>0&&l.metadata_bytes<=d.metadata_bytes&&l.profile_bytes>0&&l.profile_bytes<=d.profile_bytes&&l.exif_bytes>0&&l.exif_bytes<=d.exif_bytes&&l.scratch_bytes>0&&l.scratch_bytes<=d.scratch_bytes&&l.context_bytes>0&&l.context_bytes<=d.context_bytes,1);
    need(l.width>0&&l.width<=d.width&&l.height>0&&l.height<=d.height&&l.marker_count>0&&l.marker_count<=d.marker_count&&l.frame_count==1,1);
}
uint64_t multiply(uint64_t a,uint64_t b){need(!b||a<=UINT64_MAX/b,8);return a*b;}
void layout(pixaura_decode_metadata& m,const pixaura_decode_limits& l){
    need(m.width>0&&m.height>0);need(m.width<=l.width&&m.height<=l.height,8);
    const auto pixels=multiply(m.width,m.height);need(pixels<=l.pixel_count,8);
    m.row_stride=multiply(m.width,4);need(m.row_stride<=l.row_stride,8);m.decoded_bytes=multiply(m.row_stride,m.height);need(m.decoded_bytes<=l.decoded_bytes&&m.decoded_bytes<=SIZE_MAX,8);
    need(multiply(pixels,4)==m.decoded_bytes);m.api_version=1;m.struct_size=sizeof(m);m.pixel_format=1;m.frame_count=1;
    m.display_width=m.orientation>=5?m.height:m.width;m.display_height=m.orientation>=5?m.width:m.height;
}
Metadata admit(const uint8_t* bytes,std::size_t n,const pixaura_decode_limits& l){
    validate_limits(l);need(n<=l.encoded_bytes,8);need(bytes!=nullptr||n==0,1);View v{bytes,n};
    if(!n)need(false,18);
    const char sig[]="\x89PNG\r\n\x1a\n";
    if(n<8&&std::memcmp(bytes,sig,n)==0)need(false,18);
    if(v.is(0,sig,8))return png(v,l);
    if(v.at(0)==0xff){need(n>=2,18);if(v.at(1)==0xd8)return jpeg(v,l);}
    if(v.is(0,"GIF8",4)||v.is(0,"RIFF",4)||v.is(4,"ftyp",4)||v.is(0,"II",2)||v.is(0,"MM",2)||v.is(0,"BM",2))need(false,16);
    need(false);return {};
}
#ifdef PIXAURA_DECODE_TESTING
thread_local int allocation_fail_after=-1;
thread_local int execution_fault=0;
#endif
std::unique_ptr<Vector<uint8_t>> execute(const Source& source,const pixaura_decode_limits& l){
    auto m=source.metadata().value;layout(m,l);auto pixels=std::make_unique<Vector<uint8_t>>(static_cast<std::size_t>(m.decoded_bytes));int failure=-1,fault=0;
#ifdef PIXAURA_DECODE_TESTING
    failure=allocation_fail_after;fault=execution_fault;
#endif
    const auto status=pixaura_codec_decode(m.format,source.encoded().data(),source.encoded().size(),pixels->data(),m.width,m.height,static_cast<std::size_t>(m.row_stride),static_cast<std::size_t>(l.scratch_bytes),failure,fault);
    need(status==0,status);return pixels;
}
}
