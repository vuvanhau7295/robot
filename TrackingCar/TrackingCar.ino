#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <Arduino.h>

MDNSResponder mdns;
const char *ssid = "Minh Anh";
const char *password = "06021995";

ESP8266WebServer server(80);
ESP8266HTTPUpdateServer updater(true);

const int RIGHT_MOTOR_1 = D2;
const int RIGHT_MOTOR_2 = D3;
const int RIGHT_MOTOR_CONTROL = D4;

const int LEFT_MOTOR_1 = D5;
const int LEFT_MOTOR_2 = D6;
const int LEFT_MOTOR_CONTROL = D7;

int speed = 1000;

const int BACKWARD = -1;
const int FORWARD = 1;

void setLeftMotorDirection(int direction) {
	if (direction == BACKWARD) {
		digitalWrite(LEFT_MOTOR_1, HIGH);
		digitalWrite(LEFT_MOTOR_2, LOW);
	} else if (direction == FORWARD) {
		digitalWrite(LEFT_MOTOR_1, LOW);
		digitalWrite(LEFT_MOTOR_2, HIGH);
	} else {
		digitalWrite(LEFT_MOTOR_1, LOW);
		digitalWrite(LEFT_MOTOR_2, LOW);
	}

}

void setLeftMotorSpeed(int speed) {
	if (speed > 0) {
		setLeftMotorDirection(FORWARD);
		analogWrite(LEFT_MOTOR_CONTROL, speed);
	} else if (speed < 0) {
		setLeftMotorDirection(BACKWARD);
		analogWrite(LEFT_MOTOR_CONTROL, -speed);
	} else {
		analogWrite(LEFT_MOTOR_CONTROL, 0);
	}
}

void setRightMotorDirection(int direction) {
	if (direction == BACKWARD) {
		digitalWrite(RIGHT_MOTOR_1, HIGH);
		digitalWrite(RIGHT_MOTOR_2, LOW);
	} else if (direction == FORWARD) {
		digitalWrite(RIGHT_MOTOR_1, LOW);
		digitalWrite(RIGHT_MOTOR_2, HIGH);
	} else {
		digitalWrite(RIGHT_MOTOR_1, LOW);
		digitalWrite(RIGHT_MOTOR_2, LOW);
	}
}

void setRightMotorSpeed(int speed) {
	if (speed > 0) {
		setRightMotorDirection(FORWARD);
		analogWrite(RIGHT_MOTOR_CONTROL, speed);
	} else if (speed < 0) {
		setRightMotorDirection(BACKWARD);
		analogWrite(RIGHT_MOTOR_CONTROL, -speed);
	} else {
		analogWrite(RIGHT_MOTOR_CONTROL, 0);
	}
}

void stop() {
	setLeftMotorSpeed(0);
	setRightMotorSpeed(0);
}

void forward() {
	setLeftMotorSpeed(speed);
	setRightMotorSpeed(speed);
}

void right() {
	setLeftMotorSpeed(speed);
	setRightMotorSpeed(-speed);
}

void right2() {
	setLeftMotorSpeed(speed);
	setRightMotorSpeed(0);
}

void left() {
	setLeftMotorSpeed(-speed);
	setRightMotorSpeed(speed);
}

void left2() {
	setLeftMotorSpeed(0);
	setRightMotorSpeed(speed);
}

void backward() {
	setLeftMotorSpeed(-speed);
	setRightMotorSpeed(-speed);
}

void echoWebpage() {
	server.send(200, "text/html",
			(String) "<html><a href='/'><h1>Color tracking car control panel <small>v2.1.4</small></h1></a>"
					"<h3>Speed</h3>" + "<p><form method='get' action='/speed'>"
					"<input type='text' name='speed' value='" + speed
					+ "'><input type='submit' value='Update'></p>"
							"</form>"
							"<h3>Movement</h3>"
					+ "____<a href=\"forward\"><button>Forward</button></a><br/>"
							"<a href=\"left\"><button>Left</button></a>"
							"<a href=\"stop\"><button>Stop</button></a>"
							"<a href=\"right\"><button>Right</button></a><br/>"
							"____<a href=\"backward\"><button>Backward</button></a>"
							"<h3>Update firmware</h3>"
							"<form method='POST' action='/update' enctype='multipart/form-data'><input type='file' name='update'> <input type='submit' value='Update'></form>"
							"</html>");
}

long timeout = 0;

void setTimeout(int duration = 200) {
	if (server.hasArg("duration"))
		timeout = millis() + server.arg("duration").toInt();
	else
		timeout = millis() + duration;
}

void echoSensor() {
	server.send(200, "text/json", "Done");
}

void motors_handler() {
	if (server.hasArg("command")) {
		String command = server.arg("command");
		if (command == "forward") {
			forward();
		} else if (command == "backward") {
			backward();
		} else if (command == "left") {
			if (server.hasArg("mode2"))
				left2();
			else
				left();
		} else if (command == "right") {
			if (server.hasArg("mode2"))
				right2();
			else
				right();
		}
		setTimeout();
	}
	echoSensor();
}

void setup(void) {
	pinMode(LED_BUILTIN, OUTPUT);
	digitalWrite(LED_BUILTIN, HIGH);
	pinMode(LEFT_MOTOR_1, OUTPUT);
	pinMode(LEFT_MOTOR_2, OUTPUT);
	pinMode(RIGHT_MOTOR_1, OUTPUT);
	pinMode(RIGHT_MOTOR_2, OUTPUT);
	pinMode(RIGHT_MOTOR_CONTROL, OUTPUT);
	pinMode(LEFT_MOTOR_CONTROL, OUTPUT);
	stop();
	Serial.begin(115200);
	WiFi.begin(ssid, password);
	Serial.print("Connecting.");

	// Wait for connection
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}
	Serial.println("");
	Serial.print("Connected to ");
	Serial.println(ssid);
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());

	if (mdns.begin("esp8266", WiFi.localIP())) {
		Serial.println("MDNS responder started");
	}

	server.on("/", []() {
		echoWebpage();
	});

	server.on("/forward", []() {
		forward();
		setTimeout();
		echoWebpage();
	});
	server.on("/left", []() {
		left();
		setTimeout();
		echoWebpage();
	});
	server.on("/right", []() {
		right();
		setTimeout();
		echoWebpage();
	});
	server.on("/backward", []() {
		backward();
		setTimeout();
		echoWebpage();
	});
	server.on("/stop", []() {
		stop();
		echoWebpage();
	});
	server.on("/test", []() {
		server.send(200, "text/html", "ESP8266");
	});

	server.on("/speed", []() {
		String _speed = server.arg("speed");
		if (_speed != "") {
			speed = _speed.toInt();
		}
		echoWebpage();
	});
	server.on("/move", motors_handler);
	updater.setup(&server);
	server.begin();
	digitalWrite(LED_BUILTIN, LOW);
	Serial.println("HTTP server started :)");
}

void loop(void) {
	server.handleClient();
	if (millis() > timeout)
		stop();
}

