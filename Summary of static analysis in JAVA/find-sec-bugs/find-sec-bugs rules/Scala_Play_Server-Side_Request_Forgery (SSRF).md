###Scala Play Server-Side Request Forgery (SSRF)

Scala Play服务器端请求伪造（SSRF）

**Bug Pattern：**SCALA_PLAY_SSRF
####描述：
服务器端请求伪造发生在Web服务器执行对用户提供的未经验证的目标参数的请求时。 此类漏洞可能允许攻击者访问内部服务或从Web服务器发起攻击。

- [x] 安全相关

####漏洞代码：
```
def doGet(value:String) = Action {

    WS.url(value).get().map { response =>

        Ok(response.body)

    }

}

```
####解决方案：

不要接受来自用户的URL目标

接受一个目标键，并用它来查找目标（合法）目标

白名单URL（如果可能的话）

验证URL的开头是白名单的一部分

#### Check级别：AST

####相关信息：
1. [CWE-918: Server-Side Request Forgery (SSRF)](https://cwe.mitre.org/data/definitions/918.html )
1. [Understanding Server-Side Request Forgery ](https://www.bishopfox.com/blog/2015/04/vulnerable-by-design-understanding-server-side-request-forgery/ )


