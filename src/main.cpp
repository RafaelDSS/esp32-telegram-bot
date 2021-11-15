#include <Arduino.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#define ssid_wifi "nome da rede"
#define password_wifi "senha"

// Pines
#define LED_BUILTIN 2

// Commands
#define POWER_ON_LIGHT "/poweron"
#define POWER_OFF_LIGHT "/poweroff"

#define TEMPO_ENTRE_CHECAGEM_DE_MENSAGENS 100 //ms

#define token_acesso_telegram "token"

WiFiClientSecure client;
UniversalTelegramBot bot(token_acesso_telegram, client);
unsigned long timestamp_checagem_msg_telegram = 0;
int num_mensagens_recebidas_telegram = 0;
String resposta_msg_recebida;

void init_wifi(void);
void conecta_wifi(void);
void verifica_conexao_wifi(void);
unsigned long diferenca_tempo(unsigned long timestamp_referencia);
String trata_mensagem_recebida(String msg_recebida);

void init_wifi(void)
{
  Serial.println("------WI-FI -----");
  Serial.print("Conectando-se a rede: ");
  Serial.println(ssid_wifi);
  Serial.println("Aguarde...");
  conecta_wifi();
}

void conecta_wifi(void)
{
  if (WiFi.status() == WL_CONNECTED)
    return;

  WiFi.begin(ssid_wifi, password_wifi);

  while (WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(100 / portTICK_PERIOD_MS);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("Conectado com sucesso a rede wi-fi ");
  Serial.println(ssid_wifi);
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());
  client.setInsecure();
}

void verifica_conexao_wifi(void)
{
  conecta_wifi();
}

unsigned long diferenca_tempo(unsigned long timestamp_referencia)
{
  return (millis() - timestamp_referencia);
}

String trata_mensagem_recebida(String msg_recebida)
{
  String resposta = "";
  bool comando_valido = false;

  if (msg_recebida.equals(POWER_ON_LIGHT))
  {
    digitalWrite(LED_BUILTIN, HIGH);
    resposta = "LED Ligado!";
    comando_valido = true;
  }

  if (msg_recebida.equals(POWER_OFF_LIGHT))
  {
    digitalWrite(LED_BUILTIN, LOW);
    resposta = "LED Desligado!";
    comando_valido = true;
  }

  if (comando_valido == false)
    resposta = "Comando invalido: " + msg_recebida;

  return resposta;
}

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);

  init_wifi();

  timestamp_checagem_msg_telegram = millis();
}

void loop()
{
  int i;

  verifica_conexao_wifi();

  if (diferenca_tempo(timestamp_checagem_msg_telegram) >= TEMPO_ENTRE_CHECAGEM_DE_MENSAGENS)
  {
    num_mensagens_recebidas_telegram = bot.getUpdates(bot.last_message_received + 1);

    if (num_mensagens_recebidas_telegram > 0)
    {
      Serial.print("[BOT] Mensagens recebidas: ");
      Serial.println(num_mensagens_recebidas_telegram);
    }
    while (num_mensagens_recebidas_telegram)
    {
      for (i = 0; i < num_mensagens_recebidas_telegram; i++)
      {
        resposta_msg_recebida = "";
        resposta_msg_recebida = trata_mensagem_recebida(bot.messages[i].text);
        bot.sendMessage(bot.messages[i].chat_id, resposta_msg_recebida, "");
      }

      num_mensagens_recebidas_telegram = bot.getUpdates(bot.last_message_received + 1);
    }

    timestamp_checagem_msg_telegram = millis();
  }
}
