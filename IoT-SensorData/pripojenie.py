import paho.mqtt.client as mqtt
import mysql.connector
import json


db = mysql.connector.connect(
    host="localhost",
    user="root",          
    password="databaza",  
    database="meteostanica"
)

cursor = db.cursor()


def on_message(client, userdata, msg):
    try:
        
        data = json.loads(msg.payload.decode())
        teplota = data['temperature']
        vlhkost = data['humidity']
        tlak = data['pressure']
        svietivost = data['light_level']
        zrazky = data['rain_status']

        
        sql = "INSERT INTO sensor_data (teplota, vlhkost, tlak, svietivost, zrazky) VALUES (%s, %s, %s, %s, %s)"
        values = (teplota, vlhkost, tlak, svietivost, zrazky)
        cursor.execute(sql, values)
        db.commit()

        print("Údaje uložené do databázy:", values)
    except Exception as e:
        print("Chyba pri spracovaní správy:", e)


client = mqtt.Client()
client.on_message = on_message

broker = "192.168.***.***"  
client.connect(broker, 1883, 60)


client.subscribe("sensor/data")

print("Pripojený na MQTT broker, čakám na dáta...")
client.loop_forever()

