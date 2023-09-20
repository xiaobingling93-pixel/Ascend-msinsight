### 重构记录
代码重构涉及的改动较多，该文件用于记录发现的问题和在重构过程中使用的通用方法，
用于解决问题和避免问题再次出现。

1. 使用函数式接口，移除handler中的RequestHandler，并简化代码  
***注意事项***：  
使用策略模式，将要分发的任务添加到一个HashMap中  
Handler层应该只做任务的分发和回调，实际的业务逻辑应当放到Service层  
Handler层最终的结果应该都只包含类似DeleteSessionHandler中的三部分，如果还有其它代码，则可能包含了业务逻辑，需要进一步分析和优化   
参考MR地址：https://codehub-g.huawei.com/IDE-DS/Profiler/ohos-hiinsight/merge_requests/1910
2. JsVmExportHandler.java中存在大量相似的方法，可以通过将不同的部分提取出来作为入参，合并为一个方法，且将参数添加到一个map中，遍历执行  
DataField, DataType, DataMask存在着映射关系，最好使用一个枚举类型合并，即DataEnum.java(由于DataFile等类在其它代码中还有用到，尚未整改，
所以没有删除，整改完成后可以删除)  
DataMask有移位和16进制两种形式，在DataEnum中统一使用移位的形式表示，更加直观  
参考MR地址： https://codehub-g.huawei.com/IDE-DS/Profiler/ohos-hiinsight/merge_requests/1920
3. 重构过程中，新需求的代码尽量保证依赖的类都在同一个包内，不要跨包依赖；为了方便重构，如果新的功能特性代码没有合适的位置可以放，
那就新建包、类来承载，不要将新功能和无关的老功能代码耦合在一起
4. 提供自定义工具类JsonUtil,用于json反序列化时进行校验
使用如下：
入参为JSONObject或者json字符串，要转成一个DTO,DTO定义如下  
```java
@Data
public class SessionDTO{
    @NotBlank
    private String sessionId;
    @NotNull
    private Long startTime;
}
```
在字段上面添加了@NotBlank和@NotNull注解  
在使用JsonUtil.parseObjectWithValidate方法将入参转换成SessionDto实体类时  
就会校验sessionId是否为blank,startTime是否为null(注意，Long使用包装类型，不能用long)  
当前实现较基础，后续根据需要可自定义拓展新的注解和在ValidateUtil中添加对应的校验逻辑  
因为使用了反射，在超高频场景下使用需验证下是否会影响性能  
参考MR地址： https://codehub-g.huawei.com/IDE-DS/Profiler/ohos-hiinsight/merge_requests/1981
5. 避免Optional的滥用，Optional是为了告诉函数的调用者，函数的返回值中存在没有结果的可能性  
如果函数必然是有返回值的，则建议不要使用Optional，会让代码变得复杂
参考MR地址： https://codehub-g.huawei.com/IDE-DS/Profiler/ohos-hiinsight/merge_requests/1986
