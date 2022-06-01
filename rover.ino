/*By Md. Saleh Sadiq Tanim*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Servo.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>

ESP8266WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

Servo servo; //camera rotation
Servo servo_hor;
Servo servo_ver;
Servo servo_pick;

StaticJsonDocument<192> doc;

#define ssid "OPTIMUS-Access-Point"
#define password "123opop123"

#define servo_pin D5
#define servo_hor_pin D6
#define servo_ver_pin D7
#define servo_pick_pin D8

#define left_motor1 D1
#define left_motor2 D2
#define right_motor1 D3
#define right_motor2 D4


String str_payload = "";
bool toggle_rotation = false;
unsigned long now_time = 0;

int servo_x_val = 0;
bool flag_x = true;

const char *html_content = "<!DOCTYPE html>\n"
                           "<html lang=\"en\">\n"
                           "\n"
                           "<head>\n"
                           "    <meta charset=\"UTF-8\">\n"
                           "    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\n"
                           "    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
                           "    <title>Rover Controller</title>\n"
                           "\n"
                           "    <style>\n"
                           "        .center{\n"
                           "            margin: 0 auto;\n"
                           "            text-align: center;\n"
                           "        }\n"
                           "        .bottom{\n"
                           "        \tmargin-top: 2rem;\n"
                           "            width: 99%;\n"
                           "        }\n"
                           "        #car-control{\n"
                           "            padding-top: 2rem;\n"
                           "        }\n"
                           "        #arm-control{\n"
                           "            padding-top: 5rem;\n"
                           "        }\n"
                           "\n"
                           "        .btn{\n"
                           "            background-color: rgb(252, 72, 72);\n"
                           "            border: none;\n"
                           "            outline: none;\n"
                           "            width: 5rem;\n"
                           "            height: 5rem;\n"
                           "            border-radius: 50%;\n"
                           "            transition: .2s;\n"
                           "        }\n"
                           "        .btn:active{\n"
                           "        \ttransform: scale(1.2);\n"
                           "            background-color: rgb(197, 34, 34);;\n"
                           "            border-radius: 30%;\n"
                           "        }\n"
                           "        \n"
                           "        \n"
                           "      #wrap {\n"
                           "        margin: auto;\n"
                           "        width: 100%;\n"
                           "        height: 13rem;\n"
                           "        padding: 0;\n"
                           "        overflow: hidden;\n"
                           "      }\n"
                           "      #scaled-frame {\n"
                           "        width: 1600px;\n"
                           "        height: 1200px;\n"
                           "        border: 0px;\n"
                           "      }\n"
                           "      #scaled-frame {\n"
                           "        zoom: 0.25;\n"
                           "        -moz-transform: scale(0.25);\n"
                           "        -moz-transform-origin: 0 0;\n"
                           "        -o-transform: scale(0.25);\n"
                           "        -o-transform-origin: 0 0;\n"
                           "        -webkit-transform: scale(0.25);\n"
                           "        -webkit-transform-origin: 0 0;\n"
                           "      }\n"
                           "      @media screen and (-webkit-min-device-pixel-ratio:0) {\n"
                           "        #scaled-frame {\n"
                           "          zoom: 1;\n"
                           "        }\n"
                           "      }\n"
                           "      \n"
                           "    </style>\n"
                           "</head>\n"
                           "\n"
                           "<body>\n"
                           "  <div id=\"wrap\">\n"
                           "   <iframe src=\"http://192.168.4.1/\" id=\"scaled-frame\">\n"
                           "     </iframe> \n"
                           "  </div>\n"
                           "\t \n"
                           "    <div class=\"center\" style=\"margin-top: 2rem;\">\n"
                           "        <button onclick=\"Arm_Controller()\" style=\"margin-right: 5rem;\">\n"
                           "            <b>Arm Controller</b>\n"
                           "        </button>\n"
                           "        \n"
                           "        <button onclick=\"Car_Controller()\">\n"
                           "            <b>Car Controller</b>\n"
                           "        </button>\n"
                           "    </div>\n"
                           "\n"
                           "    <div id=\"car-control\" class=\"center\" style=\"display: block;\">\n"
                           "        <button class=\"btn\" ontouchstart=\"move_car('forward')\" ontouchend=\"move_car('stop')\"></button> <br>\n"
                           "        <button class=\"btn\" style=\"margin-right: 5rem;\" ontouchstart=\"move_car('left')\" ontouchend=\"move_car('stop')\"></button>\n"
                           "        <button class=\"btn\" ontouchstart=\"move_car('right')\" ontouchend=\"move_car('stop')\"></button> <br>\n"
                           "        <button class=\"btn\" ontouchstart=\"move_car('backward')\" ontouchend=\"move_car('stop')\"></button>\n"
                           "    </div>\n"
                           "\n"
                           "    <div id=\"arm-control\" class=\"center\" style=\"display: none;\">\n"
                           "        Vertical <br>\n"
                           "        <input type=\"range\" min=\"1000\" max=\"2000\" value=\"1500\" oninput=\"servo_vertical(this.value)\"> <br>\n"
                           "        Horizontal <br>\n"
                           "        <input type=\"range\" min=\"1000\" max=\"2000\" value=\"1500\" oninput=\"servo_horizontal(this.value)\"> <br>\n"
                           "        Picker <br>\n"
                           "        <input type=\"range\" min=\"1000\" max=\"2000\" value=\"1500\" oninput=\"servo_picker(this.value)\"> <br>\n"
                           "        <button onclick=\"auto_rotate()\" style=\"margin-top: 3rem;\">\n"
                           "          Auto Rotate\n"
                           "        </button>\n"
                           "\n"
                           "    </div>\n"
                           "\n"
                           "    <div class=\"center bottom\">\n"
                           "        <b>Camera Controller</b> <br>\n"
                           "        <input type=\"range\" min=\"1000\" max=\"2000\" value=\"1500\" \n"
                           "        style=\"width: 16rem;\" oninput=\"camera_movement(this.value)\">\n"
                           "    </div>\n"
                           "\n"
                           "<script>\n"
                           "    let car_ctrl = document.getElementById(\"car-control\");\n"
                           "    let arm_ctrl = document.getElementById(\"arm-control\");\n"
                           "    let Socket = new WebSocket('ws://' + window.location.hostname + ':81/');\n"
                           "    \n"
                           "    //init value    \n"
                           "    let json = {\n"
                           "    \t\t\"direction\": \"stop\",\n"
                           "        \"camera_rot\" : 1500,\n"
                           "        \"servo_vertical\": 1500,\n"
                           "        \"servo_horizontal\": 1500,\n"
                           "        \"servo_picker\": 1500\n"
                           "    \t};\n"
                           "    \n"
                           "    function auto_rotate(){\n"
                           "      var xmlHttp = new XMLHttpRequest();\n"
                           "      xmlHttp.open(\"GET\", \"/rotate\", true);\n"
                           "      xmlHttp.send(null);\n"
                           "    }\n"
                           "    \n"
                           "    function Arm_Controller(){\n"
                           "        car_ctrl.style.display = \"none\";\n"
                           "        arm_ctrl.style.display = \"block\";\n"
                           "    }\n"
                           "    function Car_Controller(){\n"
                           "        car_ctrl.style.display = \"block\";\n"
                           "        arm_ctrl.style.display = \"none\";\n"
                           "    }\n"
                           "    \n"
                           "    function camera_movement(val){\n"
                           "      json[\"camera_rot\"] = parseInt(val);\n"
                           "    \tSocket.send(JSON.stringify(json));\n"
                           "    }\n"
                           "    function move_car(dir){\n"
                           "    \tjson[\"direction\"] = dir;\n"
                           "        Socket.send(JSON.stringify(json));\n"
                           "    }\n"
                           "    function servo_vertical(val){\n"
                           "    \tjson[\"servo_vertical\"] = parseInt(val);\n"
                           "    \tSocket.send(JSON.stringify(json));\n"
                           "    }\n"
                           "    function servo_horizontal(val){\n"
                           "    \tjson[\"servo_horizontal\"] = parseInt(val);\n"
                           "    \tSocket.send(JSON.stringify(json));\n"
                           "    }\n"
                           "    function servo_picker(val){\n"
                           "    \tjson[\"servo_picker\"] = parseInt(val);\n"
                           "    \tSocket.send(JSON.stringify(json));\n"
                           "    }\n"
                           "</script>\n"
                           "</body>\n"
                           "</html>";
void main_page() {
  server.send(200, "text/html", html_content);
}

void auto_rotate() {
  toggle_rotation = !toggle_rotation;
  server.send(200, "text/plain", "ok");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    str_payload = String((char *)payload);
    deserializeJson(doc, str_payload);

    if (!toggle_rotation) servo.writeMicroseconds(doc["camera_rot"]);

    servo_hor.writeMicroseconds(doc["servo_horizontal"]);
    servo_ver.writeMicroseconds(doc["servo_vertical"]);
    servo_pick.writeMicroseconds(doc["servo_picker"]);


    if (doc["direction"] != "stop") {
      if (doc["direction"] == "forward") forward();
      else if (doc["direction"] == "right") right();
      else if (doc["direction"] == "left") left();
      else if (doc["direction"] == "backward") backward();
    }
    else _stop_();
  }
}


void setup() {

  servo.attach(servo_pin);
  servo_hor.attach(servo_hor_pin);
  servo_ver.attach(servo_ver_pin);
  servo_pick.attach(servo_pick_pin);

  pinMode(left_motor1, OUTPUT);
  pinMode(left_motor2, OUTPUT);
  pinMode(right_motor1, OUTPUT);
  pinMode(right_motor2, OUTPUT);

  servo.write(90);
  servo_hor.write(90);
  servo_ver.write(90);
  servo_pick.write(90);
  
  delay(300);

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", main_page);
  server.on("/rotate", auto_rotate);

  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  Serial.println("Server listening");
}


void rotate_servo_x() {
  if (servo_x_val >= 180) {
    servo_x_val = 180;
    flag_x = false;
  }
  else if (servo_x_val <= 0) {
    servo_x_val = 0;
    flag_x = true;
  }
  if (flag_x) {
    servo_x_val += 5;
  }
  else {
    servo_x_val -= 5;
  }
  servo.write(servo_x_val);
}



/*   <movement>    */

void left() {
  digitalWrite(left_motor1, HIGH);
  digitalWrite(left_motor2, LOW);
  digitalWrite(right_motor1, LOW);
  digitalWrite(right_motor2, HIGH);

}

void right() {
  digitalWrite(left_motor1, LOW);
  digitalWrite(left_motor2, HIGH);
  digitalWrite(right_motor1, HIGH);
  digitalWrite(right_motor2, LOW);
}

void forward() {
  digitalWrite(left_motor1, HIGH);
  digitalWrite(left_motor2, LOW);
  digitalWrite(right_motor1, HIGH);
  digitalWrite(right_motor2, LOW);
}

void backward() {
  digitalWrite(left_motor1, LOW);
  digitalWrite(left_motor2, HIGH);
  digitalWrite(right_motor1, LOW);
  digitalWrite(right_motor2, HIGH);
}

void _stop_() {
  digitalWrite(left_motor1, LOW);
  digitalWrite(left_motor2, LOW);
  digitalWrite(right_motor1, LOW);
  digitalWrite(right_motor2, LOW);

}

/*    </movement>   */

void loop() {
  webSocket.loop();
  server.handleClient();

  if (toggle_rotation) {
    if ((millis() - now_time) > 350) {
      rotate_servo_x();
      now_time = millis();
    }
  }
}
