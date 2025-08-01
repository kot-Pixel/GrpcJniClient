// Generated by the gRPC C++ plugin.
// If you make any local change, they will be lost.
// source: rfcomm.protoc
#ifndef GRPC_rfcomm_2eprotoc__INCLUDED
#define GRPC_rfcomm_2eprotoc__INCLUDED

#include "rfcomm.protoc.pb.h"

#include <functional>
#include <grpcpp/generic/async_generic_service.h>
#include <grpcpp/support/async_stream.h>
#include <grpcpp/support/async_unary_call.h>
#include <grpcpp/support/client_callback.h>
#include <grpcpp/client_context.h>
#include <grpcpp/completion_queue.h>
#include <grpcpp/support/message_allocator.h>
#include <grpcpp/support/method_handler.h>
#include <grpcpp/impl/proto_utils.h>
#include <grpcpp/impl/rpc_method.h>
#include <grpcpp/support/server_callback.h>
#include <grpcpp/impl/server_callback_handlers.h>
#include <grpcpp/server_context.h>
#include <grpcpp/impl/service_type.h>
#include <grpcpp/support/status.h>
#include <grpcpp/support/stub_options.h>
#include <grpcpp/support/sync_stream.h>
#include <grpcpp/ports_def.inc>

namespace carplay {
namespace bt {

class CarplayBtService final {
 public:
  static constexpr char const* service_full_name() {
    return "carplay.bt.CarplayBtService";
  }
  class StubInterface {
   public:
    virtual ~StubInterface() {}
    std::unique_ptr< ::grpc::ClientReaderWriterInterface< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>> RfcommTransport(::grpc::ClientContext* context) {
      return std::unique_ptr< ::grpc::ClientReaderWriterInterface< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>>(RfcommTransportRaw(context));
    }
    std::unique_ptr< ::grpc::ClientAsyncReaderWriterInterface< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>> AsyncRfcommTransport(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderWriterInterface< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>>(AsyncRfcommTransportRaw(context, cq, tag));
    }
    std::unique_ptr< ::grpc::ClientAsyncReaderWriterInterface< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>> PrepareAsyncRfcommTransport(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderWriterInterface< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>>(PrepareAsyncRfcommTransportRaw(context, cq));
    }
    virtual ::grpc::Status StartBtIap2Link(::grpc::ClientContext* context, const ::google::protobuf::Empty& request, ::carplay::bt::StartBtIap2LinkResponse* response) = 0;
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::carplay::bt::StartBtIap2LinkResponse>> AsyncStartBtIap2Link(::grpc::ClientContext* context, const ::google::protobuf::Empty& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::carplay::bt::StartBtIap2LinkResponse>>(AsyncStartBtIap2LinkRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::carplay::bt::StartBtIap2LinkResponse>> PrepareAsyncStartBtIap2Link(::grpc::ClientContext* context, const ::google::protobuf::Empty& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReaderInterface< ::carplay::bt::StartBtIap2LinkResponse>>(PrepareAsyncStartBtIap2LinkRaw(context, request, cq));
    }
    class async_interface {
     public:
      virtual ~async_interface() {}
      virtual void RfcommTransport(::grpc::ClientContext* context, ::grpc::ClientBidiReactor< ::carplay::bt::RfcommPacket,::carplay::bt::RfcommPacket>* reactor) = 0;
      virtual void StartBtIap2Link(::grpc::ClientContext* context, const ::google::protobuf::Empty* request, ::carplay::bt::StartBtIap2LinkResponse* response, std::function<void(::grpc::Status)>) = 0;
      virtual void StartBtIap2Link(::grpc::ClientContext* context, const ::google::protobuf::Empty* request, ::carplay::bt::StartBtIap2LinkResponse* response, ::grpc::ClientUnaryReactor* reactor) = 0;
    };
    typedef class async_interface experimental_async_interface;
    virtual class async_interface* async() { return nullptr; }
    class async_interface* experimental_async() { return async(); }
   private:
    virtual ::grpc::ClientReaderWriterInterface< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>* RfcommTransportRaw(::grpc::ClientContext* context) = 0;
    virtual ::grpc::ClientAsyncReaderWriterInterface< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>* AsyncRfcommTransportRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) = 0;
    virtual ::grpc::ClientAsyncReaderWriterInterface< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>* PrepareAsyncRfcommTransportRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::carplay::bt::StartBtIap2LinkResponse>* AsyncStartBtIap2LinkRaw(::grpc::ClientContext* context, const ::google::protobuf::Empty& request, ::grpc::CompletionQueue* cq) = 0;
    virtual ::grpc::ClientAsyncResponseReaderInterface< ::carplay::bt::StartBtIap2LinkResponse>* PrepareAsyncStartBtIap2LinkRaw(::grpc::ClientContext* context, const ::google::protobuf::Empty& request, ::grpc::CompletionQueue* cq) = 0;
  };
  class Stub final : public StubInterface {
   public:
    Stub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());
    std::unique_ptr< ::grpc::ClientReaderWriter< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>> RfcommTransport(::grpc::ClientContext* context) {
      return std::unique_ptr< ::grpc::ClientReaderWriter< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>>(RfcommTransportRaw(context));
    }
    std::unique_ptr<  ::grpc::ClientAsyncReaderWriter< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>> AsyncRfcommTransport(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderWriter< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>>(AsyncRfcommTransportRaw(context, cq, tag));
    }
    std::unique_ptr<  ::grpc::ClientAsyncReaderWriter< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>> PrepareAsyncRfcommTransport(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncReaderWriter< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>>(PrepareAsyncRfcommTransportRaw(context, cq));
    }
    ::grpc::Status StartBtIap2Link(::grpc::ClientContext* context, const ::google::protobuf::Empty& request, ::carplay::bt::StartBtIap2LinkResponse* response) override;
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::carplay::bt::StartBtIap2LinkResponse>> AsyncStartBtIap2Link(::grpc::ClientContext* context, const ::google::protobuf::Empty& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::carplay::bt::StartBtIap2LinkResponse>>(AsyncStartBtIap2LinkRaw(context, request, cq));
    }
    std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::carplay::bt::StartBtIap2LinkResponse>> PrepareAsyncStartBtIap2Link(::grpc::ClientContext* context, const ::google::protobuf::Empty& request, ::grpc::CompletionQueue* cq) {
      return std::unique_ptr< ::grpc::ClientAsyncResponseReader< ::carplay::bt::StartBtIap2LinkResponse>>(PrepareAsyncStartBtIap2LinkRaw(context, request, cq));
    }
    class async final :
      public StubInterface::async_interface {
     public:
      void RfcommTransport(::grpc::ClientContext* context, ::grpc::ClientBidiReactor< ::carplay::bt::RfcommPacket,::carplay::bt::RfcommPacket>* reactor) override;
      void StartBtIap2Link(::grpc::ClientContext* context, const ::google::protobuf::Empty* request, ::carplay::bt::StartBtIap2LinkResponse* response, std::function<void(::grpc::Status)>) override;
      void StartBtIap2Link(::grpc::ClientContext* context, const ::google::protobuf::Empty* request, ::carplay::bt::StartBtIap2LinkResponse* response, ::grpc::ClientUnaryReactor* reactor) override;
     private:
      friend class Stub;
      explicit async(Stub* stub): stub_(stub) { }
      Stub* stub() { return stub_; }
      Stub* stub_;
    };
    class async* async() override { return &async_stub_; }

   private:
    std::shared_ptr< ::grpc::ChannelInterface> channel_;
    class async async_stub_{this};
    ::grpc::ClientReaderWriter< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>* RfcommTransportRaw(::grpc::ClientContext* context) override;
    ::grpc::ClientAsyncReaderWriter< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>* AsyncRfcommTransportRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq, void* tag) override;
    ::grpc::ClientAsyncReaderWriter< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>* PrepareAsyncRfcommTransportRaw(::grpc::ClientContext* context, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::carplay::bt::StartBtIap2LinkResponse>* AsyncStartBtIap2LinkRaw(::grpc::ClientContext* context, const ::google::protobuf::Empty& request, ::grpc::CompletionQueue* cq) override;
    ::grpc::ClientAsyncResponseReader< ::carplay::bt::StartBtIap2LinkResponse>* PrepareAsyncStartBtIap2LinkRaw(::grpc::ClientContext* context, const ::google::protobuf::Empty& request, ::grpc::CompletionQueue* cq) override;
    const ::grpc::internal::RpcMethod rpcmethod_RfcommTransport_;
    const ::grpc::internal::RpcMethod rpcmethod_StartBtIap2Link_;
  };
  static std::unique_ptr<Stub> NewStub(const std::shared_ptr< ::grpc::ChannelInterface>& channel, const ::grpc::StubOptions& options = ::grpc::StubOptions());

  class Service : public ::grpc::Service {
   public:
    Service();
    virtual ~Service();
    virtual ::grpc::Status RfcommTransport(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>* stream);
    virtual ::grpc::Status StartBtIap2Link(::grpc::ServerContext* context, const ::google::protobuf::Empty* request, ::carplay::bt::StartBtIap2LinkResponse* response);
  };
  template <class BaseClass>
  class WithAsyncMethod_RfcommTransport : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_RfcommTransport() {
      ::grpc::Service::MarkMethodAsync(0);
    }
    ~WithAsyncMethod_RfcommTransport() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status RfcommTransport(::grpc::ServerContext* /*context*/, ::grpc::ServerReaderWriter< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>* /*stream*/)  override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestRfcommTransport(::grpc::ServerContext* context, ::grpc::ServerAsyncReaderWriter< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>* stream, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncBidiStreaming(0, context, stream, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithAsyncMethod_StartBtIap2Link : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithAsyncMethod_StartBtIap2Link() {
      ::grpc::Service::MarkMethodAsync(1);
    }
    ~WithAsyncMethod_StartBtIap2Link() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status StartBtIap2Link(::grpc::ServerContext* /*context*/, const ::google::protobuf::Empty* /*request*/, ::carplay::bt::StartBtIap2LinkResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestStartBtIap2Link(::grpc::ServerContext* context, ::google::protobuf::Empty* request, ::grpc::ServerAsyncResponseWriter< ::carplay::bt::StartBtIap2LinkResponse>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  typedef WithAsyncMethod_RfcommTransport<WithAsyncMethod_StartBtIap2Link<Service > > AsyncService;
  template <class BaseClass>
  class WithCallbackMethod_RfcommTransport : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_RfcommTransport() {
      ::grpc::Service::MarkMethodCallback(0,
          new ::grpc::internal::CallbackBidiHandler< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>(
            [this](
                   ::grpc::CallbackServerContext* context) { return this->RfcommTransport(context); }));
    }
    ~WithCallbackMethod_RfcommTransport() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status RfcommTransport(::grpc::ServerContext* /*context*/, ::grpc::ServerReaderWriter< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>* /*stream*/)  override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerBidiReactor< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>* RfcommTransport(
      ::grpc::CallbackServerContext* /*context*/)
      { return nullptr; }
  };
  template <class BaseClass>
  class WithCallbackMethod_StartBtIap2Link : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithCallbackMethod_StartBtIap2Link() {
      ::grpc::Service::MarkMethodCallback(1,
          new ::grpc::internal::CallbackUnaryHandler< ::google::protobuf::Empty, ::carplay::bt::StartBtIap2LinkResponse>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::google::protobuf::Empty* request, ::carplay::bt::StartBtIap2LinkResponse* response) { return this->StartBtIap2Link(context, request, response); }));}
    void SetMessageAllocatorFor_StartBtIap2Link(
        ::grpc::MessageAllocator< ::google::protobuf::Empty, ::carplay::bt::StartBtIap2LinkResponse>* allocator) {
      ::grpc::internal::MethodHandler* const handler = ::grpc::Service::GetHandler(1);
      static_cast<::grpc::internal::CallbackUnaryHandler< ::google::protobuf::Empty, ::carplay::bt::StartBtIap2LinkResponse>*>(handler)
              ->SetMessageAllocator(allocator);
    }
    ~WithCallbackMethod_StartBtIap2Link() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status StartBtIap2Link(::grpc::ServerContext* /*context*/, const ::google::protobuf::Empty* /*request*/, ::carplay::bt::StartBtIap2LinkResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* StartBtIap2Link(
      ::grpc::CallbackServerContext* /*context*/, const ::google::protobuf::Empty* /*request*/, ::carplay::bt::StartBtIap2LinkResponse* /*response*/)  { return nullptr; }
  };
  typedef WithCallbackMethod_RfcommTransport<WithCallbackMethod_StartBtIap2Link<Service > > CallbackService;
  typedef CallbackService ExperimentalCallbackService;
  template <class BaseClass>
  class WithGenericMethod_RfcommTransport : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_RfcommTransport() {
      ::grpc::Service::MarkMethodGeneric(0);
    }
    ~WithGenericMethod_RfcommTransport() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status RfcommTransport(::grpc::ServerContext* /*context*/, ::grpc::ServerReaderWriter< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>* /*stream*/)  override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithGenericMethod_StartBtIap2Link : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithGenericMethod_StartBtIap2Link() {
      ::grpc::Service::MarkMethodGeneric(1);
    }
    ~WithGenericMethod_StartBtIap2Link() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status StartBtIap2Link(::grpc::ServerContext* /*context*/, const ::google::protobuf::Empty* /*request*/, ::carplay::bt::StartBtIap2LinkResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
  };
  template <class BaseClass>
  class WithRawMethod_RfcommTransport : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_RfcommTransport() {
      ::grpc::Service::MarkMethodRaw(0);
    }
    ~WithRawMethod_RfcommTransport() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status RfcommTransport(::grpc::ServerContext* /*context*/, ::grpc::ServerReaderWriter< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>* /*stream*/)  override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestRfcommTransport(::grpc::ServerContext* context, ::grpc::ServerAsyncReaderWriter< ::grpc::ByteBuffer, ::grpc::ByteBuffer>* stream, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncBidiStreaming(0, context, stream, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawMethod_StartBtIap2Link : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawMethod_StartBtIap2Link() {
      ::grpc::Service::MarkMethodRaw(1);
    }
    ~WithRawMethod_StartBtIap2Link() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status StartBtIap2Link(::grpc::ServerContext* /*context*/, const ::google::protobuf::Empty* /*request*/, ::carplay::bt::StartBtIap2LinkResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    void RequestStartBtIap2Link(::grpc::ServerContext* context, ::grpc::ByteBuffer* request, ::grpc::ServerAsyncResponseWriter< ::grpc::ByteBuffer>* response, ::grpc::CompletionQueue* new_call_cq, ::grpc::ServerCompletionQueue* notification_cq, void *tag) {
      ::grpc::Service::RequestAsyncUnary(1, context, request, response, new_call_cq, notification_cq, tag);
    }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_RfcommTransport : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_RfcommTransport() {
      ::grpc::Service::MarkMethodRawCallback(0,
          new ::grpc::internal::CallbackBidiHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context) { return this->RfcommTransport(context); }));
    }
    ~WithRawCallbackMethod_RfcommTransport() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status RfcommTransport(::grpc::ServerContext* /*context*/, ::grpc::ServerReaderWriter< ::carplay::bt::RfcommPacket, ::carplay::bt::RfcommPacket>* /*stream*/)  override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerBidiReactor< ::grpc::ByteBuffer, ::grpc::ByteBuffer>* RfcommTransport(
      ::grpc::CallbackServerContext* /*context*/)
      { return nullptr; }
  };
  template <class BaseClass>
  class WithRawCallbackMethod_StartBtIap2Link : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithRawCallbackMethod_StartBtIap2Link() {
      ::grpc::Service::MarkMethodRawCallback(1,
          new ::grpc::internal::CallbackUnaryHandler< ::grpc::ByteBuffer, ::grpc::ByteBuffer>(
            [this](
                   ::grpc::CallbackServerContext* context, const ::grpc::ByteBuffer* request, ::grpc::ByteBuffer* response) { return this->StartBtIap2Link(context, request, response); }));
    }
    ~WithRawCallbackMethod_StartBtIap2Link() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable synchronous version of this method
    ::grpc::Status StartBtIap2Link(::grpc::ServerContext* /*context*/, const ::google::protobuf::Empty* /*request*/, ::carplay::bt::StartBtIap2LinkResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    virtual ::grpc::ServerUnaryReactor* StartBtIap2Link(
      ::grpc::CallbackServerContext* /*context*/, const ::grpc::ByteBuffer* /*request*/, ::grpc::ByteBuffer* /*response*/)  { return nullptr; }
  };
  template <class BaseClass>
  class WithStreamedUnaryMethod_StartBtIap2Link : public BaseClass {
   private:
    void BaseClassMustBeDerivedFromService(const Service* /*service*/) {}
   public:
    WithStreamedUnaryMethod_StartBtIap2Link() {
      ::grpc::Service::MarkMethodStreamed(1,
        new ::grpc::internal::StreamedUnaryHandler<
          ::google::protobuf::Empty, ::carplay::bt::StartBtIap2LinkResponse>(
            [this](::grpc::ServerContext* context,
                   ::grpc::ServerUnaryStreamer<
                     ::google::protobuf::Empty, ::carplay::bt::StartBtIap2LinkResponse>* streamer) {
                       return this->StreamedStartBtIap2Link(context,
                         streamer);
                  }));
    }
    ~WithStreamedUnaryMethod_StartBtIap2Link() override {
      BaseClassMustBeDerivedFromService(this);
    }
    // disable regular version of this method
    ::grpc::Status StartBtIap2Link(::grpc::ServerContext* /*context*/, const ::google::protobuf::Empty* /*request*/, ::carplay::bt::StartBtIap2LinkResponse* /*response*/) override {
      abort();
      return ::grpc::Status(::grpc::StatusCode::UNIMPLEMENTED, "");
    }
    // replace default version of method with streamed unary
    virtual ::grpc::Status StreamedStartBtIap2Link(::grpc::ServerContext* context, ::grpc::ServerUnaryStreamer< ::google::protobuf::Empty,::carplay::bt::StartBtIap2LinkResponse>* server_unary_streamer) = 0;
  };
  typedef WithStreamedUnaryMethod_StartBtIap2Link<Service > StreamedUnaryService;
  typedef Service SplitStreamedService;
  typedef WithStreamedUnaryMethod_StartBtIap2Link<Service > StreamedService;
};

}  // namespace bt
}  // namespace carplay


#include <grpcpp/ports_undef.inc>
#endif  // GRPC_rfcomm_2eprotoc__INCLUDED
