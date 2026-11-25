#ifndef PIXAURA_PUBLICATION_V2_HPP
#define PIXAURA_PUBLICATION_V2_HPP
#include "document.hpp"
namespace pixaura::commerce {class CommercialCoordinatorV2;}
namespace pixaura::editor {class EditorContextV2;}
namespace pixaura::storage {
using document::String;
// Trusted native OS/logical-clock boundary, not UI or receipt-import input.
struct PublicationTimeV2 {
    int64_t logical_utc=0, observed_wall_utc=0, uptime_seconds=0;
    String boot;
    bool uncertain=false, utc_authenticated=false;
};
struct PublicationClockV2 {virtual ~PublicationClockV2()=default;virtual PublicationTimeV2 sample_at_publication()=0;};
// Implemented by the app-owned protected installation key. Never use a public
// digest as an authenticator. Test authorities are explicitly synthetic.
struct ReceiptAuthorityV2 {
    virtual ~ReceiptAuthorityV2()=default;
    virtual String authenticate(std::string_view canonical)=0;
};
class Repository;
// Successful, usable advisory delivery is a separate native event. It is NOT
// an edit receipt, approval or executable capability and cannot start trial.
class NativeAdvisoryDeliveryV2 final {
    friend class pixaura::editor::EditorContextV2;
    friend class pixaura::commerce::CommercialCoordinatorV2;
    String event_,source_,project_,document_,revision_,session_,context_;
    String token_,capsule_,operations_digest_,seal_;
    uint64_t epoch_=0,generation_=0;
    bool validated_at_fence_=false;
    NativeAdvisoryDeliveryV2()=default;
    String canonical()const{
        String out("PIXAURA_NATIVE_USABLE_ADVISORY_DELIVERY_V2");
        const auto add=[&](std::string_view s){out+=':';out.append(std::to_string(s.size()));out+=':';out.append(s);};
        for(const auto* value:{&event_,&source_,&project_,&document_,&revision_,&session_,&context_,&token_,&capsule_,&operations_digest_})add(*value);
        add(std::to_string(epoch_));add(std::to_string(generation_));return out;
    }
};
class NativeCommitReceiptV2 final {
    friend class Repository;
    friend class pixaura::commerce::CommercialCoordinatorV2;
    String event_,source_,project_,document_,revision_,parent_,session_,kind_,seal_,prior_seal_,revision_digest_;
    uint64_t bytes_=0,epoch_=0,ordinal_=0;
    PublicationTimeV2 time_;
    bool validated_after_commit_=false;
    NativeCommitReceiptV2()=default;
public:
    const String& event()const{return event_;}
    const String& source()const{return source_;}
    const String& project()const{return project_;}
    const String& document()const{return document_;}
    const String& revision()const{return revision_;}
    const String& parent()const{return parent_;}
    const String& kind()const{return kind_;}
    const String& revision_digest()const{return revision_digest_;}
    uint64_t bytes()const{return bytes_;}
    uint64_t epoch()const{return epoch_;}
    const PublicationTimeV2& time()const{return time_;}
    // Length-delimited domain-separated fields; no serialized Plan/capability.
    String canonical()const{
        String out("PIXAURA_NATIVE_COMMIT_RECEIPT_V2");
        const auto add=[&](std::string_view value){out+=':';out.append(std::to_string(value.size()));out+=':';out.append(value);};
        for(const auto* value:{&event_,&source_,&project_,&document_,&revision_,&parent_,&session_,&kind_,&prior_seal_,&revision_digest_})add(*value);
        for(const auto& value:{std::to_string(bytes_),std::to_string(epoch_),std::to_string(ordinal_),std::to_string(time_.logical_utc),std::to_string(time_.observed_wall_utc),std::to_string(time_.uptime_seconds)})add(value);
        add(time_.boot);add(time_.uncertain?"1":"0");add(time_.utc_authenticated?"1":"0");return out;
    }
};
}
#endif
