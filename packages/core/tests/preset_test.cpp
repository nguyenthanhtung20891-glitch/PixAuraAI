#include "../src/preset.hpp"
#include "../src/evaluation.hpp"
#include "pixaura/preset.h"
#include <fstream>
#include <iterator>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <new>
#include <exception>
#include <string>
#include <limits>
// Test-only one-shot allocation failure; no allocator hook ships.
static thread_local int64_t fail_after = -1;
static thread_local bool injected = false;


void* operator new(std::size_t size) {
    if (fail_after == 0) { fail_after = -1; injected = true; throw std::bad_alloc(); }
    if (fail_after > 0) --fail_after;
    if (void* pointer = std::malloc(size ? size : 1)) return pointer;
    throw std::bad_alloc();
}
void* operator new[](std::size_t size) { return ::operator new(size); }
void operator delete(void* pointer) noexcept { std::free(pointer); }
void operator delete[](void* pointer) noexcept { std::free(pointer); }
void operator delete(void* pointer, std::size_t) noexcept { std::free(pointer); }
void operator delete[](void* pointer, std::size_t) noexcept { std::free(pointer); }

using namespace pixaura;
static unsigned checks=0;
#define CHECK(x) do{++checks;if(!(x)){std::fprintf(stderr,"preset check %d: %s\n",__LINE__,#x);std::exit(1);}}while(false)
static std::string id(unsigned n){char b[33];std::snprintf(b,sizeof(b),"%032x",n);return b;}
static const uint8_t* ptr(const std::string& s){return reinterpret_cast<const uint8_t*>(s.data());}
static std::string op(unsigned n,const std::string& tool,const std::string& parameter,int value){return "{\"id\":\""+id(n)+"\",\"type\":\"pixaura."+tool+"\",\"operation_version\":1,\"parameter_version\":1,\"parameters\":{\""+parameter+"\":"+std::to_string(value)+"}}";}
static std::string recipe(const std::string& operations,const std::string& name="reference.test"){return "{\"schema_version\":1,\"recipe_version\":1,\"preset_id\":\""+name+"\",\"name\":\"Reference only\",\"operations\":["+operations+"]}";}
static std::string bindings(unsigned first,unsigned count){std::string s="{\"operation_ids\":[";for(unsigned i=0;i<count;++i){if(i)s+=',';s+='"';s+=id(first+i);s+='"';}return s+"]}";}
static document::Snapshot good(document::Result<document::Snapshot> r){CHECK(r.code==0);return r.value;}
static document::Snapshot navigation(const document::Snapshot& s,const char* kind){const std::string command="{\"command_version\":1,\"kind\":\""+std::string(kind)+"\",\"expected_revision_id\":\""+s->current().text()+"\",\"expected_session_id\":\""+s->session().id.text()+"\",\"expected_generation\":"+std::to_string(s->session().generation)+"}";return good(document::transition(*s,command));}
int main(int argc,char** argv){CHECK(argc==2);std::ifstream input(argv[1],std::ios::binary);const std::string manifest{std::istreambuf_iterator<char>(input),{}};const auto session=id(500),cid=id(600);const auto base=good(document::deserialize(manifest,session));
 const auto one=recipe(op(1,"exposure","milli_ev",1000)),multi=recipe(op(1,"brightness","milli_linear",100)+","+op(2,"blur","milli_strength",500)+","+op(3,"exposure","milli_ev",0));
 for(const auto& text:{one,multi}){auto parsed=document::parse_preset(text);CHECK(parsed.code==0);auto canonical=document::serialize_preset(parsed.value);CHECK(canonical.code==0&&canonical.value.back()=='\n');CHECK(document::serialize_preset(document::parse_preset(canonical.value).value).value==canonical.value);}
 for(const auto& invalid:{recipe(""),recipe(op(1,"crop","x_ppm",0)),recipe(op(1,"denoise","amount",1)),recipe(op(1,"exposure","milli_ev",5001)),recipe(op(1,"blur","milli_strength",1)+","+op(1,"blur","milli_strength",2)),recipe(op(1,"exposure","milli_ev",1),"local..bad"),std::string("{}"),multi+"x"})CHECK(document::parse_preset(invalid).code!=0);
 for(std::size_t i=0;i<multi.size();++i)CHECK(document::parse_preset(std::string_view(multi).substr(0,i)).code!=0);
 for(const auto* field:{"schema_version","recipe_version","operation_version","parameter_version"}){auto bad=one;auto at=bad.find(std::string(field)+"\":1");bad.replace(at+std::strlen(field)+2,1,"2");CHECK(document::parse_preset(bad).code==(std::strlen(field)<18&&std::string(field).find("operation")==std::string::npos&&std::string(field).find("parameter")==std::string::npos?4:5));}
 auto duplicate=one;duplicate.insert(1,"\"schema_version\":1,");CHECK(document::parse_preset(duplicate).code==6);
 auto escaped=one;escaped.insert(1,"\"schema_\\u0076ersion\":1,");CHECK(document::parse_preset(escaped).code==6);
 CHECK(document::parse_preset(std::string(16385,' ')).code==8);
 std::string oversized;for(unsigned i=0;i<17;++i){if(i)oversized+=',';oversized+=op(i+1,"blur","milli_strength",1);}CHECK(document::parse_preset(recipe(oversized)).code==8);
 auto parsed=document::parse_preset(multi);CHECK(parsed.code==0);preset::Catalog catalog({parsed.value});CHECK(catalog.recipes().size()==1);bool duplicated=false;try{preset::Catalog bad({parsed.value,parsed.value});}catch(const document::Failure& e){duplicated=e.code==6;}CHECK(duplicated);
 document::Vector<document::PresetRecipe> too_many(17,parsed.value);bool bounded=false;try{preset::Catalog bad(too_many);}catch(const document::Failure& e){bounded=e.code==8;}CHECK(bounded);
 auto escaped_name=one;escaped_name.replace(escaped_name.find("Reference only"),1,"\\u0052");CHECK(document::serialize_preset(document::parse_preset(escaped_name).value).value==document::serialize_preset(document::parse_preset(one).value).value);
 std::string maximum;for(unsigned i=0;i<16;++i){if(i)maximum+=',';maximum+=op(i+1,"blur","milli_strength",1);}CHECK(document::parse_preset(recipe(maximum)).code==0);
 auto first_recipe=parsed.value,second_recipe=parsed.value;first_recipe.preset_id="local.a";second_recipe.preset_id="local.b";preset::Catalog stable({second_recipe,first_recipe});CHECK(stable.recipes()[0].preset_id=="local.a");
 for(const auto& descriptor:manual::registry())if(descriptor.category=="tone_color"||descriptor.category=="detail"){
    const auto& parameter=descriptor.parameters[0];const auto tool=std::string(descriptor.type.substr(8)),key=std::string(parameter.name);
    for(const int value:{parameter.minimum,parameter.neutral,parameter.maximum})CHECK(document::parse_preset(recipe(op(1,tool,key,value))).code==0);
    CHECK(document::parse_preset(recipe(op(1,tool,key,parameter.minimum-1))).code==7);CHECK(document::parse_preset(recipe(op(1,tool,key,parameter.maximum+1))).code==7);
 }
 const auto binding=bindings(700,3),revision=id(800);auto a=good(preset::propose(base,base,multi,binding,revision));CHECK(a->revisions().size()==base->revisions().size()+1&&a->operations().size()==base->operations().size()+3&&a->source().sha256==base->source().sha256);
 auto reference=document::replay(*base,base->current());CHECK(reference.code==0);document::Vector<document::Id> stack;for(const auto& item:reference.value)stack.push_back(item.id);auto operations=parsed.value.operations;for(unsigned i=0;i<3;++i){operations[i].id=document::Id::parse(id(700+i));stack.push_back(operations[i].id);}document::DetachedCandidate manual{base->identity(),base->current(),base->session(),stack,operations,document::Id::parse(revision),"manual",std::nullopt};auto identical=good(document::transition(*base,manual));CHECK(document::serialize(*a).value==document::serialize(*identical).value);
 working::Image source;source.metadata={1,sizeof(pixaura_working_metadata),3,2,1,1,1,0,48,96};source.pixels=std::make_unique<decode::Vector<float>>(24,0.25f);for(unsigned i=3;i<24;i+=4)(*source.pixels)[i]=1.0f;
 const auto ordered=document::replay(*a,a->current()).value;auto pixels=evaluation::evaluate(source,ordered,working::defaults());auto sequential=evaluation::evaluate(source,{},working::defaults());for(const auto& item:ordered)sequential=evaluation::evaluate(sequential,{item},working::defaults());CHECK(*pixels.pixels==*sequential.pixels);
 for(const auto& text:{one,multi}){auto b=good(preset::propose(a,a,text,bindings(900, text==one?1:3),id(950)));CHECK(b->revisions().size()==a->revisions().size()+1);auto undone=navigation(b,"undo"),redone=navigation(undone,"redo");CHECK(undone->current()==a->current()&&redone->current()==b->current());CHECK(document::replay(*redone,redone->current()).value.size()==document::replay(*b,b->current()).value.size());CHECK(preset::propose(a,undone,text,bindings(1000,text==one?1:3),id(1050)).code==9);}
 CHECK(good(preset::propose(base,base,recipe(op(1,"exposure","milli_ev",0)),bindings(700,1),id(800)))==base);
 CHECK(preset::propose(base,base,multi,bindings(17,3),revision).code==6);CHECK(preset::propose(base,base,multi,bindings(700,2),revision).code==6);
 auto other_session=good(document::deserialize(manifest,id(501)));CHECK(preset::propose(base,other_session,multi,binding,revision).code==9);
 auto replaced_source=manifest;replaced_source.replace(replaced_source.find(std::string(64,'a')),64,std::string(64,'b'));CHECK(preset::propose(base,good(document::deserialize(replaced_source,session)),multi,binding,revision).code==9);
 (*source.pixels)[0]=std::numeric_limits<float>::max();bool pixel_failed=false;try{evaluation::evaluate(source,document::parse_preset(one).value.operations,working::defaults());}catch(const document::Failure& e){pixel_failed=e.code==7;}CHECK(pixel_failed&&(*source.pixels)[0]==std::numeric_limits<float>::max());
 pixaura_document_context context{};pixaura_document_handle handle{};CHECK(pixaura_document_context_init(1,&context,sizeof(context),ptr(cid),32,nullptr)==0);CHECK(pixaura_document_open(1,&context,ptr(manifest),manifest.size(),ptr(session),32,&handle,nullptr)==0);
 uint64_t required=991;CHECK(pixaura_preset_catalog(2,nullptr,0,&required)==2&&required==991);CHECK(pixaura_preset_catalog(1,nullptr,0,&required)==11);std::string empty(static_cast<std::size_t>(required),' ');CHECK(pixaura_preset_catalog(1,reinterpret_cast<uint8_t*>(empty.data()),empty.size(),&required)==0&&empty=="{\"presets\":[]}\n");
 unsigned failures=0;for(int path=0;path<2;++path){bool success=false;for(int64_t n=0;n<6000;++n){pixaura_document_handle out=handle;uint32_t changed=991;uint64_t needed=991;uint8_t output[16384];std::memset(output,0x5a,sizeof(output));fail_after=n;injected=false;int rc=path==0?pixaura_preset_canonical(1,ptr(multi),multi.size(),output,sizeof(output),&needed):pixaura_preset_propose(1,&context,&handle,&handle,ptr(multi),multi.size(),ptr(binding),binding.size(),ptr(revision),32,&out,&changed);fail_after=-1;
  if(injected){CHECK(rc==8);++failures;if(path==0)CHECK(needed==991&&output[0]==0x5a);else CHECK(out.serial==handle.serial&&changed==991);}else{CHECK(rc==0);if(path==1){CHECK(changed==1&&out.serial!=handle.serial);CHECK(pixaura_document_release(&context,&out)==0);}success=true;break;}}
 CHECK(success);}

 // A preset's constituent remains editable by the accepted ordinary gesture path.
 auto edit=manual::Gesture::begin(a,id(1200),"pixaura.brightness",id(700));CHECK(edit.code==0);
 const std::string update="{\"operations\":["+op(1201,"brightness","milli_linear",200)+"]}";
 CHECK(edit.value->update(update).code==0);auto edited=good(edit.value->commit(a,id(1202)));
 const auto edited_stack=document::replay(*edited,edited->current());CHECK(edited_stack.code==0&&edited_stack.value.size()==ordered.size());
 CHECK(edited_stack.value[reference.value.size()].id.text()==id(1201));
 auto after_manual=good(preset::propose(edited,edited,one,bindings(1300,1),id(1301)));CHECK(document::replay(*after_manual,after_manual->current()).value.back().id.text()==id(1300));
 // Existing 256-operation cap rejects a whole batch with no partial history.
 auto full=base;for(unsigned i=0;i<254;++i)full=good(preset::propose(full,full,one,bindings(2000+i,1),id(4000+i)));
 CHECK(document::replay(*full,full->current()).value.size()==256);CHECK(preset::propose(full,full,one,bindings(6000,1),id(6001)).code==8);
 // Shared handle admission also preserves every output; explicit release permits retry.
 pixaura_document_handle held[63];for(auto& h:held)CHECK(pixaura_document_open(1,&context,ptr(manifest),manifest.size(),ptr(session),32,&h,nullptr)==0);
 pixaura_document_handle out=handle;uint32_t changed=991;CHECK(pixaura_preset_propose(1,&context,&handle,&handle,ptr(multi),multi.size(),ptr(binding),binding.size(),ptr(revision),32,&out,&changed)==8&&out.serial==handle.serial&&changed==991);
 CHECK(pixaura_document_release(&context,&held[62])==0);CHECK(pixaura_preset_propose(1,&context,&handle,&handle,ptr(multi),multi.size(),ptr(binding),binding.size(),ptr(revision),32,&out,&changed)==0&&changed==1);CHECK(pixaura_document_release(&context,&out)==0);for(unsigned i=0;i<62;++i)CHECK(pixaura_document_release(&context,&held[i])==0);
 // No executable payload, geometry, oversized metadata or duplicate binding is tolerated.
 auto script=one;script.insert(1,"\"script\":\"run\",");CHECK(document::parse_preset(script).code!=0);
 CHECK(document::parse_preset(recipe(op(1,"rotate","quarter_turns",1))).code==5);
 auto long_name=one;long_name.replace(long_name.find("Reference only"),14,std::string(65,'x'));CHECK(document::parse_preset(long_name).code!=0);
 CHECK(document::parse_preset_bindings("{\"operation_ids\":[\"00000000000000000000000000000001\",\"00000000000000000000000000000001\"]}").code==6);
 CHECK(pixaura_document_context_destroy(&context)==0);std::printf("Preset foundation PASS %u checks; ordered/manual pixel equivalence, batch history, repeat/stale/parser/catalog bounds; %u allocation failures recovered\n",checks,failures);return 0;}
