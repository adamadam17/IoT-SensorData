import paho.mqtt.client as mqtt
import mysql.connector
import json

# Konfigurácia MySQL databázy
db = mysql.connector.connect(
    host="localhost",
    user="root",          # Nahraďte používateľským menom pre MySQL
    password="databaza",  # Nahraďte heslom pre MySQL
    database="meteostanica"
)

cursor = db.cursor()

# Funkcia na spracovanie prijatých dát z MQTT
def on_message(client, userdata, msg):
    try:
        # Prevod prijatých údajov z JSON
        data = json.loads(msg.payload.decode())
        teplota = data['temperature']
        vlhkost = data['humidity']
        tlak = data['pressure']
        svietivost = data['light_level']
        zrazky = data['rain_status']

        # SQL príkaz na vloženie údajov do tabuľky
        sql = "INSERT INTO sensor_data (teplota, vlhkost, tlak, svietivost, zrazky) VALUES (%s, %s, %s, %s, %s)"
        values = (teplota, vlhkost, tlak, svietivost, zrazky)
        cursor.execute(sql, values)
        db.commit()

        print("Údaje uložené do databázy:", values)
    except Exception as e:
        print("Chyba pri spracovaní správy:", e)

# Pripojenie na MQTT broker
client = mqtt.Client()
client.on_message = on_message

broker = "192.168.0.243"  # IP adresa vášho MQTT brokeru
client.connect(broker, 1883, 60)

# Prihlásenie na odber témy
client.subscribe("sensor/data")

print("Pripojený na MQTT broker, čakám na dáta...")
client.loop_forever()

