# The HTTP Flow
1.First, the browser opens a TCP connection to the server. This ensures data can be sent back and forth over the network and that the data sent from one end is put together the same way at the other end. If the connection happens over HTTPS, TLS certificates are exchanged to ensure only the computer and the server can encrypt and decrypt the transmitted data. This prevents anyone from being able to eavesdrop on the conversation between the client and the server and steal the data they are transmitting.

2.Second, the browser sends an HTTP message. This message always contains an HTTP method, like GET, PUT, DELETE or something similar, and a URL pointing at the requested resource. It can also contain headers like cookies or authentication data and data if the browser is submitting data to the server using the post, put, or path methods.

3.Third, the server performs the requested actions and sends a response back to the browser. This response will contain an HTTP status message indicating what happened, headers with information about the response, and whatever data was requested. This data could be an HTML document or a style sheet or a JavaScript file or image or any other type of content used in a standard website.


3.Finally, once the response is fully received, the TCP connection is closed. Since HTTP is stateless, we are now back to a clean slate.
