# Proyecto de Prueba: Control LED vía LoRaWAN (TTN)

## Estructura
- `frontend/`: Interfaz web (Vue.js)
- `backend/`: Servidor Node.js/Express
- `.env`: Credenciales TTN (a completar por ti)

## Pasos para usar

### 1. Clona el repositorio y entra a la carpeta

### 2. Configura tus credenciales TTN
Copia el archivo `.env.example` a `.env` en la carpeta `backend/` y rellena los datos:

```
APP_EUI=tu_app_eui
DEV_EUI=tu_dev_eui
APP_KEY=tu_app_key
TTN_REGION=eu1
TTN_APP_ID=tu_app_id
TTN_ACCESS_KEY=tu_access_key
```

### 3. Instala dependencias y ejecuta el backend
```
cd backend
npm install
npm start
```

### 4. Instala dependencias y ejecuta el frontend
```
cd ../frontend
npm install
npm run serve
```

### 5. Abre la interfaz web
Abre tu navegador en http://localhost:8080

---

## ¿Cómo funciona?
- Pulsa "Encender LED" o "Apagar LED" en la web.
- El backend recibe la orden y la envía a TTN usando tus credenciales.
- TTN reenvía el mensaje a tu Arduino WAN 1310.

---

## Notas
- Asegúrate de tener Node.js instalado.
- El backend debe estar corriendo antes de usar la web.
- El Arduino debe estar registrado en TTN y escuchando comandos. 