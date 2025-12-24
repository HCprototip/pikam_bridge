# Proyecto: Sistema SAT Remoto para Básculas PIKAM

## Resumen del trabajo realizado (Diciembre 2025)

### ✅ Fase 1 completada: Migración WiFi → BLE

**Objetivo:** Cambiar sistema de comunicación de WiFi/WebSocket a BLE para que usuarios mantengan internet.

**Cambios realizados:**

1. **Firmware ESP32:**
   - Eliminadas dependencias WiFi/WebSocket/SuperK
   - Implementado BLE Server con características TX/RX
   - UUIDs definidos: `SERVICE_UUID = 4fafc201-1fb5-459e-8fcc-c5c9c331914b`
   - Fragmentación JSON para límite MTU (512 bytes)
   - Archivos: `main.h`, `main.cpp`, `platformio.ini`

2. **Frontend Web:**
   - Migrado a Web Bluetooth API (Chrome/Edge/Opera)
   - Reconexión automática con dispositivos previamente autorizados
   - Requiere HTTPS (desplegado en Netlify)
   - Archivos: `ws.js`, `ui.html`, `scripts.js`
   - URL: [tu-sitio].netlify.app

3. **Ventajas logradas:**
   - Usuario mantiene conexión internet (WhatsApp, GPS, etc.)
   - Menor consumo energético (BLE vs WiFi)
   - Conexión automática después del primer emparejamiento
   - Sin cambios en hardware existente (miles de unidades)

**Estado:** ✅ Compilado, subido al ESP32, funcionando

---

## 💡 Fase 2 propuesta: Dashboard SAT Remoto (PENDIENTE)

### Concepto:
Permitir a equipo SAT controlar remotamente las básculas cuando usuarios tienen problemas de calibración.

### Arquitectura propuesta:

```
[Báscula RF] → [ESP32 BLE] ←BLE→ [Móvil Usuario] ←Internet→ [Servidor Cloud]
                                   Web App                         ↓
                                  (Netlify)                   [Dashboard SAT]
                                                              • Lista dispositivos
                                                              • Control remoto
```

### Flujo de funcionamiento:

1. **Usuario conecta:**
   - Abre web app desde móvil
   - Se conecta al ESP32 por BLE (como ahora)
   - App reporta al servidor: "Dispositivo ESP32-XXX online"

2. **Dashboard SAT (web protegida):**
   - Lista de todos los dispositivos online en tiempo real
   - Click en uno → Abre interfaz de control remoto
   - Interfaz idéntica a la del usuario (botones, visor, etc.)

3. **Control remoto:**
   - SAT pulsa botones → Comando va al servidor
   - Servidor envía al móvil del usuario
   - Móvil reenvía comando al ESP32 por BLE
   - Respuesta vuelve por el mismo camino
   - SAT ve resultado en tiempo real

### Componentes necesarios:

#### Backend (Node.js + WebSocket):
```javascript
// Tracking de dispositivos online
const onlineDevices = new Map();

// Usuario registra su dispositivo
ws.on('register', (data) => {
  onlineDevices.set(data.deviceId, {
    socketId: ws.id,
    name: data.deviceName,
    lastSeen: Date.now()
  });
  broadcastToSAT({ type: 'device-online', device: data });
});

// SAT envía comando
ws.on('command', (cmd) => {
  const targetDevice = onlineDevices.get(cmd.deviceId);
  targetDevice.socket.send(cmd);
});
```

#### Frontend SAT:
- `dashboard-sat.html` - Lista de dispositivos online
- `remote-control.html` - Control remoto (clone de ui.html)
- Login con autenticación (JWT)

#### Modificación app usuario (ws.js):
```javascript
// Conectar a servidor cloud además de BLE
const cloudWs = new WebSocket('wss://servidor-sat.com');

// Reportar cuando conecta BLE
cloudWs.send({
  type: 'register',
  deviceId: bleDevice.id,
  deviceName: bleDevice.name
});

// Escuchar comandos remotos del SAT
cloudWs.onmessage = (msg) => {
  if (msg.type === 'command') {
    SendBLE(msg.command); // Reenviar al ESP32
  }
};

// Enviar respuestas de ESP32 al SAT
function HandleBLENotification(data) {
  Rebut(data); // Procesamiento local
  cloudWs.send({ type: 'data', data }); // También al SAT
}
```

### Hosting propuesto:

**Testing (gratis):**
- Frontend: Netlify (archivos estáticos)
- Backend: Railway.app o Render.com (Node.js + WebSocket)
- 500 horas/mes gratis - suficiente para pruebas

**Producción:**
- Frontend: Netlify (sigue igual)
- Backend: Servidor empresa (mismo código Node.js)
- Solo cambiar URL del WebSocket en 1 línea

### Ventajas del sistema:

✅ SAT ayuda remotamente sin desplazarse
✅ Usuario no nota nada (transparente, solo abre la app)
✅ Registro de todas las intervenciones
✅ Escalable a miles de dispositivos simultáneos
✅ Dashboard muestra estado de toda la flota
✅ Reducción costes desplazamientos SAT
✅ Soporte 24/7 posible

### Tecnologías a usar:

- **Backend:** Node.js + Express + Socket.io (WebSocket)
- **Frontend SAT:** HTML/CSS/JS + Socket.io-client
- **Autenticación:** JWT (JSON Web Tokens)
- **Base datos (opcional):** Redis para tracking en memoria, PostgreSQL para logs
- **Hosting testing:** Railway.app (deploy desde Git automático)
- **Hosting producción:** Servidor empresa (Linux + Node.js + PM2)

### Comparación con tecnologías antiguas:

| Antes (PHP)                | Ahora (Node.js)           |
|----------------------------|---------------------------|
| Apache + PHP + MySQL       | Node.js + WebSocket       |
| Subir archivos por FTP     | Git push → Deploy auto    |
| AJAX polling cada 5s       | WebSocket (tiempo real)   |
| Configurar servidor manual | Railway auto-detecta todo |
| cPanel para DB             | 1 clic para DB            |

---

## 📝 Notas técnicas importantes:

### Limitaciones BLE actuales:
- Rango: ~10-30 metros (no es problema, móvil del usuario hace de bridge)
- MTU: 512 bytes (ya implementada fragmentación)
- Conexión 1 a 1 (suficiente para este caso)

### Seguridad a implementar (Fase 2):
- Autenticación SAT con JWT
- Registro de todas las acciones remotas
- Timeout de sesiones inactivas
- Encriptación WebSocket (WSS)

### Escalabilidad:
- Arquitectura actual soporta miles de dispositivos simultáneos
- Redis para tracking en memoria (muy rápido)
- PostgreSQL solo para logs históricos (opcional)

---

## 🚀 Próximos pasos (cuando se retome):

1. **Preparar servidor Node.js básico:**
   - WebSocket server
   - Registro de dispositivos
   - Broadcast a dashboard SAT

2. **Crear dashboard SAT:**
   - Login page
   - Lista de dispositivos online
   - Interfaz de control remoto

3. **Modificar ws.js (app usuario):**
   - Añadir conexión al servidor cloud
   - Reportar registro/desconexión
   - Proxy de comandos SAT → ESP32

4. **Testing con Railway:**
   - Deploy automático desde GitHub
   - Probar con dispositivos reales
   - Verificar latencia aceptable

5. **Migración a producción:**
   - Servidor empresa (Linux + Docker)
   - Configuración PM2 para alta disponibilidad
   - Backup y logs

---

## 📦 Archivos del proyecto actual:

### Firmware ESP32:
- `src/main.h` - Declaraciones, UUIDs BLE, funciones inline
- `src/main.cpp` - Callbacks BLE, setup, loop
- `platformio.ini` - Configuración build, macros globales

### Frontend Web:
- `data/www/ui.html` - Interfaz usuario
- `data/www/js/ws.js` - Lógica BLE, reconexión automática
- `data/www/js/scripts.js` - Manejo UI
- `data/www/estil.css` - Estilos
- `data/www/netlify.toml` - Config deploy Netlify

### Documentación:
- `README_BLE_MIGRATION.md` - Guía migración WiFi→BLE (si existe)

---

## 🔗 Enlaces útiles:

- **Netlify:** https://app.netlify.com
- **Railway:** https://railway.app
- **Web Bluetooth API:** https://developer.mozilla.org/en-US/docs/Web/API/Web_Bluetooth_API
- **Socket.io docs:** https://socket.io/docs/

---

## Contacto y contexto:

- **Empresa:** [Tu empresa]
- **Dispositivos en campo:** Miles de básculas PIKAM repartidas por el país
- **Tecnología original:** RF → ESP32 → WiFi AP → Usuario móvil
- **Tecnología actual:** RF → ESP32 → BLE → Usuario móvil (con internet)
- **Fecha migración BLE:** Diciembre 2025

---

*Documento creado para retomar proyecto SAT remoto en el futuro.*
