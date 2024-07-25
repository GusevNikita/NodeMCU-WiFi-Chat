**NodeMCU-WiFi-Chat**

Этот проект представляет собой простое приложение для чата, работающее на плате NodeMCU. Приложение устанавливает Wi-Fi соединение и веб-сервер, который размещает интерфейс чата. Пользователи могут отправлять и получать сообщения через веб-страницу, причем сообщения отображаются в реальном времени. Также могут отображаться системные сообщения в специальном формате. ~~*Возможно в некоторых местах код может быть не качественным, так как проект выполнялся на скорую руки.*~~

**Особенности**

    Обмен сообщениями в реальном времени: Пользователи могут отправлять и получать сообщения в реальном времени.
    Идентификация пользователя: Каждый пользователь может установить свое имя, которое отображается вместе с его сообщениями.
    Защита от флуда: Сервер включает защиту от флуда, чтобы предотвратить слишком частую отправку сообщений пользователями.
	Системные сообщения: Администраторы могут отправлять системные сообщения, которые выделяются красным цветом.
	Очистка сообщений: Сообщения очищаются с сервера каждые 30 минут.

**Отправка системных сообщений**

Для отправки системного сообщения, выделенного красным цветом, используйте следующую ссылку:



    http://192.168.0.97/sys?message="your message"

Это добавит в чат системное сообщение с указанным текстом.

**Кастомизация**

В проекте предусмотрен CSS, который можно изменить под свои нужды. В HTML-разметке можно найти стили, определяющие внешний вид страницы чата, включая цвета фона, текста, кнопок и сообщений. Эти стили легко изменить, чтобы подстроить интерфейс под собственные предпочтения.

**Лицензия**

Этот проект лицензирован под GPL-3.0 license.

-----------------------------------------------------------------------------------------------------------------------------------------------
**NodeMCU-WiFi-Chat**

This project is a simple chat application running on a NodeMCU board. The application establishes a Wi-Fi connection and a web server that hosts the chat interface. Users can send and receive messages through a web page, with messages being displayed in real time. System messages can also be displayed in a special format. ~~*Some parts of the code might be of low quality, as the project was done hastily.*~~

**Features**

    Real-time messaging: Users can send and receive messages in real time.
    User identification: Each user can set their name, which is displayed along with their messages.
    Flood protection: The server includes flood protection to prevent users from sending messages too frequently.
    System messages: Administrators can send system messages that are highlighted in red.
    Message clearing: Messages are cleared from the server every 30 minutes.

**Sending System Messages**

To send a system message highlighted in red, use the following link:

    http://192.168.0.97/sys?message="your message"

This will add a system message with the specified text to the chat.

**Customization**

The project includes CSS that can be modified to suit your needs. The HTML layout contains styles defining the appearance of the chat page, including background colors, text colors, button styles, and message styles. These styles can be easily adjusted to customize the interface to your liking.
License

**This project is licensed under the GPL-3.0 license.**
