#include "WiFi.h"
#include "ESP32_MailClient.h"
#include "esp_camera.h"

// WiFi credentials
const char* ssid = "Shaiek";
const char* password = "01711360250";

// Email configuration
#define emailSenderAccount    "shaiekhossain2023@gmail.com"
#define emailSenderPassword   "nylc sgee tzfd rzah"  
#define smtpServer            "smtp.gmail.com"
#define smtpServerPort        465
#define emailSubject          "ESP32-CAM Photo Captured"
#define emailRecipient        "shaiekhossain2023@example.com"

// Pin definitions for RCWL-0516 and ESP32-CAM
#define RCWL_PIN 13  // Change as per your setup

// ESP32-CAM camera configuration
#define PWDN_GPIO_NUM    32
#define RESET_GPIO_NUM   -1
#define XCLK_GPIO_NUM    0
#define SIOD_GPIO_NUM    26
#define SIOC_GPIO_NUM    27

#define Y9_GPIO_NUM      35
#define Y8_GPIO_NUM      34
#define Y7_GPIO_NUM      39
#define Y6_GPIO_NUM      36
#define Y5_GPIO_NUM      21
#define Y4_GPIO_NUM      19
#define Y3_GPIO_NUM      18
#define Y2_GPIO_NUM      5
#define VSYNC_GPIO_NUM   25
#define HREF_GPIO_NUM    23
#define PCLK_GPIO_NUM    22

// SMTP email client setup
SMTPSession smtp;

void setup() {
  Serial.begin(115200);

  // Initialize WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Setup RCWL-0516
  pinMode(RCWL_PIN, INPUT);

  // Initialize the camera
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  if (psramFound()) {
    config.frame_size = FRAMESIZE_VGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_CIF;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  // Initialize camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  // Set up SMTP email settings
  smtp.debug(1);
  smtp.callback(smtpCallback);
  smtp.sethost(SMTP_HOST);
  smtp.setport(SMTP_PORT);
  smtp.setlogin(AUTHOR_EMAIL, AUTHOR_PASSWORD);
}

void loop() {
  // Check if motion is detected
  if (digitalRead(RCWL_PIN) == HIGH) {
    Serial.println("Motion detected!");

    // Capture image
    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      return;
    }

    // Prepare the email content
    SMTP_Message message;
    message.sender.name = "ESP32-CAM";
    message.sender.email = AUTHOR_EMAIL;
    message.subject = "Motion Detected: Image from ESP32-CAM";
    message.addRecipient("Recipient", RECIPIENT_EMAIL);

    // Attach the captured image
    message.addAttachBinary(fb->buf, fb->len, "image.jpg", "image/jpg");

    // Send the email
    if (!MailClient.sendMail(smtp, message)) {
      Serial.println("Error sending email, " + smtp.errorReason());
    } else {
      Serial.println("Email sent successfully!");
    }

    // Release the frame buffer
    esp_camera_fb_return(fb);
    
    delay(10000);  // Delay before checking for motion again
  }

  delay(500);
}

void smtpCallback(SMTP_Status status) {
  if (status.info()) {
    Serial.println(status.info());
  }
  if (status.success()) {
    Serial.println("Email sent successfully");
  } else {
    Serial.println("Failed to send email");
  }
}
