# SmsJsonPolling
Android app that **polls REST APIs** to get a **list of SMS to send**<br />
it supports can also **notify the server** when the SMS are **sent** and **delivered**.<br />
<br />
![](https://raw.githubusercontent.com/mbruel/SmsJsonPolling/master/pics/SmsPolling.png)


### JSON format
it's basic, [cf the example](https://github.com/mbruel/SmsJsonPolling/blob/master/example/sms.json)
<pre>
{
  "sms" : [
    {
      "id"   : 1,
      "dest" : "+33600000000",
      "msg"  : "This is a test sending a SMS from json API"
    },
    {
      "id"   : 2,
      "dest" : "+33600000000",
      "msg"  : "This is a second test sending a SMS from json API with more text so it has to be using the multipart API from Android. Still not at 160 characters so just adding some text... hopefully it's good now! :)"
    }
  ]
}
</pre>


### Server errors (authentication...)
Just send back a JSON with only the error field
<pre>
{
  "error" : "Authentication error..."
}
</pre>


### Server notification when SMS are sent and delivered
As you can see in SmsJsonPolling::_notifyServer, a POST request is sent on the URL provided for the server notification <br />
with 3 more parameters:
- msgId: the id of the sms
- type: NotificationType (0 for SENT, 1 for DELIVERED)
- status: 1 for success, 0 otherwise


### Server initialization in main.cpp
Even if **SmsJsonPolling** supports multiple server URLs (_serverUrls) I didn't do the GUI part to manage them.
Thus it is hardcoded in the main.cpp but using SmsJsonPolling::addServerUrl


### Limitation: No Service yet so using FLAG_KEEP_SCREEN_ON
As I don't know yet how to implement a Service using QtService, the App must be ON to run<br/>
The FLAG_KEEP_SCREEN_ON is used to in the Activity in order to avoid the device to go to sleep.
<pre>
public void onCreate(Bundle savedInstanceState)
{
    super.onCreate(savedInstanceState);
    getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
}
</pre>
