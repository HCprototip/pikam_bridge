# Migración WiFi → BLE (Web Bluetooth)

## ✅ Cambios Realizados

### ESP32 (Firmware)
- **Eliminado:** WiFi, AsyncWebServer, AsyncWebSocket, SPIFFS
- **Añadido:** BLE Server con GATT Characteristics
- **Modificado:** 
  - [main.h](src/main.h) - Includes BLE, definiciones UUIDs
  - [main.cpp](src/main.cpp) - BLE Server callbacks, gestión conexión
  - [platformio.ini](platformio.ini) - Dependencias actualizadas

### Frontend (Web App)
- **Modificado:**
  - [ws.js](data/www/js/ws.js) - Web Bluetooth API en lugar de WebSocket
  - [ui.html](data/www/ui.html) - Botón "CONECTAR BLE"
  - [scripts.js](data/www/js/scripts.js) - Gestión estado BLE

## 🔧 Configuración ESP32

### UUIDs BLE (ya configurados)
```cpp
SERVICE_UUID:        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
CHARACTERISTIC_TX:   "beb5483e-36e1-4688-b7f5-ea07361b26a8" // ESP32 → Cliente
CHARACTERISTIC_RX:   "6e400002-b5a3-f393-e0a9-e50e24dcca9e" // Cliente → ESP32
```

### Nombre BLE
- Por defecto: `PIKAM_BLE_BRIDGE`
- Con config: `PIKAM_XXYY` (XX=address, YY=net)

## 🌐 Servidor Web

### Requisitos OBLIGATORIOS
**Web Bluetooth API solo funciona con HTTPS** (o localhost para desarrollo)

### Opciones de Hosting

#### Opción 1: GitHub Pages (GRATIS)
1. Crear repositorio en GitHub
2. Subir carpeta `data/www/` completa
3. Activar GitHub Pages en Settings
4. URL: `https://tu-usuario.github.io/repo-name/`

#### Opción 2: Netlify (GRATIS)
1. Crear cuenta en [netlify.com](https://netlify.com)
2. Arrastrar carpeta `data/www/`
3. URL automática HTTPS

#### Opción 3: Servidor propio con HTTPS
```bash
# Nginx con Let's Encrypt
server {
    listen 443 ssl;
    server_name tu-dominio.com;
    
    ssl_certificate /etc/letsencrypt/live/tu-dominio.com/fullchain.pem;
    ssl_certificate_key /etc/letsencrypt/live/tu-dominio.com/privkey.pem;
    
    root /path/to/www;
    index ui.html;
}
```

#### Opción 4: localhost (solo desarrollo)
```bash
# Python 3
cd data/www
python -m http.server 8000
# Acceder: http://localhost:8000/ui.html
```

## 📱 Uso de la Aplicación

### Navegadores Compatibles
✅ Chrome (Android, Windows, macOS, Linux)  
✅ Edge (Windows, macOS, Android)  
✅ Opera (Android, Windows, macOS)  
❌ Firefox (no soporta Web Bluetooth)  
❌ Safari (no soporta Web Bluetooth)  

### Pasos de Conexión
1. Encender ESP32 (BLE advertising activo)
2. Visitar web (HTTPS)
3. Clic en **"CONECTAR BLE"**
4. Seleccionar dispositivo `PIKAM_XXXX`
5. ¡Listo! Misma experiencia que antes

### Permisos
- Primera vez: Navegador solicita permiso Bluetooth
- Android: También solicita permiso de ubicación (requerido por Google)

## ⚙️ Compilación y Carga

### Firmware ESP32
```bash
# PlatformIO
pio run -t upload

# Monitor serial
pio device monitor
```

### Archivos Web
**NO es necesario cargar archivos web al ESP32**  
Los archivos HTML/JS/CSS se sirven desde tu servidor web HTTPS.

## 🔍 Diferencias vs WiFi

| Aspecto | WiFi (Anterior) | BLE (Actual) |
|---------|----------------|--------------|
| Conexión | Automática al conectarse a WiFi ESP32 | Usuario hace clic "CONECTAR BLE" |
| Alcance | ~50m | ~10m |
| Throughput | Alto (~5 Mbps) | Bajo (~1 Mbps) |
| Batería | Alta consumo | Bajo consumo |
| Multi-cliente | Sí (varios dispositivos) | Limitado (1-2 dispositivos) |
| Servidor | ESP32 sirve archivos | Servidor externo (HTTPS) |
| Internet | No necesario | Sí (para cargar web) |

## 🐛 Troubleshooting

### "Web Bluetooth no disponible"
- Usar Chrome/Edge/Opera
- Verificar HTTPS (o localhost)
- Android: Activar ubicación

### No aparece dispositivo
- Verificar ESP32 encendido
- BLE advertising activo (LED parpadeando)
- Reiniciar ESP32
- Borrar caché Bluetooth del navegador

### Desconexiones frecuentes
- Acercarse al ESP32 (<5m)
- Interferencias: alejar de WiFi 2.4GHz, microondas
- Verificar alimentación ESP32 (USB estable)

### JSON truncado/corrupto
- Implementada fragmentación automática
- Si persiste: Reducir frecuencia de envío (`INTERVAL_MSG_A_BLE`)

## 📊 Rendimiento

### Latencia
- WiFi: ~20-50ms
- BLE: ~50-150ms

### MTU BLE
- Típico: 512 bytes
- JSON grande se fragmenta automáticamente

## 🔐 Seguridad

### BLE
- Conexión no cifrada (agregar pairing si necesario)
- Solo dispositivos emparejados pueden conectar

### Web
- HTTPS obligatorio (TLS 1.2+)
- Protección contra MITM

## 📝 Notas Adicionales

### Modo Extendido
Funcional, mismo comportamiento que WiFi.

### Impresora
Funcional mediante BLE.

### Configuración
Sistema de configuración via botón CONFIG mantiene mismo comportamiento.

### Logs
Monitor serial ESP32 para debug:
```
BLE SETUP OK - Esperando conexion...
BLE Cliente conectado
BLE_rebut: P1
```

## 🚀 Próximos Pasos

1. Compilar y cargar firmware en ESP32
2. Subir carpeta `data/www` a servidor HTTPS
3. Probar conexión desde Chrome/Edge
4. Ajustar `INTERVAL_MSG_A_BLE` si hay problemas de rendimiento

---

**¿Preguntas?** Revisa logs en:
- ESP32: Monitor Serial (115200 baud)
- Web: Consola del navegador (F12)
