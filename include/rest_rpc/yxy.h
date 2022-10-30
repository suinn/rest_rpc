/*
 * @Author: suinn suinn@126.com
 * @Date: 2022-10-28 17:42:13
 * @LastEditors: suinn suinn@126.com
 * @LastEditTime: 2022-10-28 17:54:08
 * @FilePath: \rest_rpc\examples\client\yxy.h
 * @Description: 这是默认设置,请设置`customMade`, 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AEt
 */
#include<iostream>
#include<typeinfo>
#include<rest_rpc.hpp>

class RemoteInteract:private rest_rpc::rpc_client
{
    public:
    using rest_rpc::rpc_client::rpc_client;
    using rest_rpc::rpc_client::call;
    using rest_rpc::rpc_client::async_call;
    using rest_rpc::rpc_client::enable_auto_reconnect;
    using rest_rpc::rpc_client::connect;

protected:
template<typename Func>
void remoteSubscribe(const std::string& signalName, Func func)
{
  auto fun = [func](string_view data){
    //要将data反序列化为函数入参
    using tuple_type = typename rest_rpc::function_traits<Func>::bare_tuple_type;
    msgpack_codec codec;
    auto paras = codec.unpack<tuple_type>(data.data(), data.size());
    std::apply(func,std::move(paras));

  };

  rest_rpc::rpc_client::subscribe(signalName,fun);
}

template<typename Func, typename Object>
typename std::enable_if<std::is_member_function_pointer<Func>::value, void>::type
void remoteSubscribe(const std::string& signalName, Func func, Object* obj)
{
  auto fun = [func](string_view data){
    //要将data反序列化为函数入参
    MemberCallHelper callHelper(func,obj);
    using tuple_type = typename rest_rpc::function_traits<Func>::bare_tuple_type;
    msgpack_codec codec;
    auto paras = codec.unpack<tuple_type>(data.data(), data.size());
    std::apply(func,std::move(paras));

  };

  rest_rpc::rpc_client::subscribe(signalName,fun);
}

private:
template<typename Func, typename Object>
class MemberCallHelper
{
    public:
    MemberCallHelper(Func func, Object* obj):
    m_obj(obj),
    m_fn(func)
    {}

    template<typename... Args>
    typename typename rest_rpc::function_traits<Func>::return_type
    operator()(Args&&... args)
    {
        auto fn = std::mem_fn(m_fn);
        return fn(m_obj, std::forward<Args>(args)...);
    }

    private:
    Object* m_obj;
    Func m_fn;
}
}
