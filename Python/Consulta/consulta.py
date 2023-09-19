from io import open
import requests
from bs4 import BeautifulSoup
import getters

lineas_comentario = 4

fichero = open("fondos.txt", "r")
# Movimiento de puntero para no leer comentarios del archivo "fondos.txt"
for i in range(0,lineas_comentario):
    lineas = fichero.readline()

# Lectura de archivo
lineas = fichero.readlines()

cripto = {}
portfolio = []
for linea in lineas:
    moneda = linea.split(" ")
    moneda[1] = moneda[1].strip()

    cripto["Nombre"] = moneda[0]
    cripto["Cantidad"] = moneda[1]
    portfolio.append(cripto.copy())

direcciones = []
fondos = []

for item in portfolio:
    string = f"https://www.google.com/finance/quote/{item.get('Nombre')}-USD?hl=es"
    direcciones.append(string)
    fondos.append(float(item.get("Cantidad")))

# Obteniendo valor de las monedas
valores = []
for criptos in direcciones:
    valores.append(getters.get_value(criptos))


# URL para el dolar
blue = "https://dolarhoy.com/cotizaciondolarblue"
dolar   = getters.get_dolar_blue(blue)

print("===========================================================")
print(f"Precio dolar\t${dolar}")
for i,v in enumerate(valores):
    print(f"Precio de {portfolio[i].get('Nombre')}\tUSD {valores[i]}")
print("===========================================================")
print("===========================================================")
for i,v in enumerate(valores):
    print(f"Cantidad de {portfolio[i].get('Nombre')}\t {portfolio[i].get('Cantidad')}")
print("===========================================================")
print("===========================================================")
sum = 0
for i,v in enumerate(valores):
    print(f"Fondos en {portfolio[i].get('Nombre')}\t${round(float(portfolio[i].get('Cantidad'))*valores[i]*dolar,1)}\t USD {round(float(portfolio[i].get('Cantidad'))*valores[i],1)}")
    sum+=round(float(portfolio[i].get('Cantidad'))*valores[i]*dolar,1)

print(f"Total:\t\t${sum}\t USD {round(sum/dolar,1)}")
print("===========================================================")
print("===========================================================")
print("Portfolio:")
for item in portfolio:
    print(item["Nombre"].upper() + " =", item["Cantidad"])


