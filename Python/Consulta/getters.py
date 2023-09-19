import requests
from bs4 import BeautifulSoup

def get_value(url):
    req = requests.get(url)

    # Comprobamos si la solicitud fue exitosa
    if req.status_code == 200:
        # Parseamos la página web con BeautifulSoup
        soup = BeautifulSoup(req.text, 'html.parser')
        
        # Encontramos el elemento que contiene el precio actual
        price_element = soup.find(class_='YMlKec fxKbKc')
        valor = price_element.get_text()
        valor = valor.replace(".","")
        valor = valor.replace(",",".")
        valor = float(valor)
        if price_element:
            return valor
        else:
            print('No se pudo encontrar el precio de la cripto en la página.')
    else:
        print('Error al hacer la solicitud HTTP.')

def get_dolar_blue(url):
    req = requests.get(url)
    # Comprobamos si la solicitud fue exitosa
    if req.status_code == 200:
        # Parseamos la página web con BeautifulSoup
        soup = BeautifulSoup(req.text, 'html.parser')
        
        # Encontramos el elemento que contiene el precio actual
        price_element = soup.find(class_='value')
        dolar_blue = price_element.get_text()
        dolar_blue = float(dolar_blue.replace("$",""))
        if price_element:
            return dolar_blue
        else:
            print('No se pudo encontrar el precio del dolar en la página.')
    else:
        print('Error al hacer la solicitud HTTP.')