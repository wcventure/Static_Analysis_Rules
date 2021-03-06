###Overly permissive CORS policy

过分宽松的CORS政策

**Bug Pattern：**PERMISSIVE_CORS
####描述：
在HTML5之前，Web浏览器强制执行相同原点策略，以确保为了让JavaScript访问网页的内容，JavaScript和网页都必须来自同一个域。 如果没有相同来源策略，恶意网站可能会提供JavaScript，使用客户凭证加载来自其他网站的敏感信息，剔除它并将其传回给攻击者。 如果定义了一个名为Access-Control-Allow-Origin的新HTTP头，则HTML5可以让JavaScript跨域访问数据。 有了这个头部，Web服务器就可以定义哪些其他域可以使用跨域请求来访问其域。 但是，在定义头部时应该谨慎，因为过分宽松的CORS策略将允许恶意应用以不恰当的方式与受害者应用进行通信，从而导致欺骗，数据盗窃，中继和其他攻击。

- [x] 安全相关

####漏洞代码：
```
response.addHeader("Access-Control-Allow-Origin", "*");

```
####解决方案：
Avoid using * as the value of the Access-Control-Allow-Origin header, which indicates that the application's data is accessible to JavaScript running on any domain.

#### Check级别：AST

####相关信息：
1. [W3C Cross-Origin Resource Sharing](https://www.w3.org/TR/cors/ )
1. [Enable Cross-Origin Resource Sharing ](http://enable-cors.org/ )




