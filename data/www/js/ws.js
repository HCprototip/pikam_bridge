console.log("-EXE BLE");

// BLE UUIDs (deben coincidir con ESP32)
const SERVICE_UUID = '4fafc201-1fb5-459e-8fcc-c5c9c331914b';
const CHARACTERISTIC_UUID_TX = 'beb5483e-36e1-4688-b7f5-ea07361b26a8'; // Recibir del ESP32
const CHARACTERISTIC_UUID_RX = '6e400002-b5a3-f393-e0a9-e50e24dcca9e'; // Enviar al ESP32

let bleDevice = null;
let bleServer = null;
let txCharacteristic = null;
let rxCharacteristic = null;
let reconnectAttempts = 0;
const MAX_RECONNECT_ATTEMPTS = 5;

// Buffer para manejar fragmentación de datos BLE
let dataBuffer = '';

// Intentar reconectar automáticamente al cargar la página
async function AutoConnectBLE() {
    try {
        // Verificar si hay dispositivos previamente autorizados
        if (!navigator.bluetooth || !navigator.bluetooth.getDevices) {
            console.log('Web Bluetooth API no soportada o getDevices no disponible');
            return;
        }
        
        const devices = await navigator.bluetooth.getDevices();
        console.log('Dispositivos autorizados encontrados:', devices.length);
        
        // Buscar dispositivo PIKAM previamente autorizado
        const pikamDevice = devices.find(device => device.name && device.name.startsWith('PIKAM'));
        
        if (pikamDevice) {
            console.log('Dispositivo PIKAM encontrado, reconectando automáticamente...');
            bleDevice = pikamDevice;
            
            // Intentar conectar
            await ConnectToDevice();
        } else {
            console.log('No hay dispositivos PIKAM previamente autorizados');
        }
    } catch (error) {
        console.log('No se pudo autoconectar:', error);
    }
}

// Conectar a un dispositivo específico (reutilizable)
async function ConnectToDevice() {
    try {
        if (!bleDevice) {
            console.error('No hay dispositivo seleccionado');
            return;
        }
        
        console.log('Conectando a:', bleDevice.name);
        
        // Conectar al servidor GATT
        bleServer = await bleDevice.gatt.connect();
        console.log('Conectado al servidor GATT');
        
        // Obtener servicio
        const service = await bleServer.getPrimaryService(SERVICE_UUID);
        console.log('Servicio obtenido');
        
        // Obtener características
        txCharacteristic = await service.getCharacteristic(CHARACTERISTIC_UUID_TX);
        rxCharacteristic = await service.getCharacteristic(CHARACTERISTIC_UUID_RX);
        console.log('Características obtenidas');
        
        // Suscribirse a notificaciones (recibir datos del ESP32)
        await txCharacteristic.startNotifications();
        txCharacteristic.addEventListener('characteristicvaluechanged', HandleBLENotification);
        
        console.log('BLE Conectado exitosamente');
        EstatConn(true);
        reconnectAttempts = 0;
        EnviaDatos();
        
        // Manejar desconexión
        bleDevice.addEventListener('gattserverdisconnected', OnBLEDisconnected);
        
    } catch (error) {
        console.error('Error conectando:', error);
        EstatConn(false);
        
        // Intentar reconectar automáticamente
        if (reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
            reconnectAttempts++;
            console.log(`Reintentando conexión (${reconnectAttempts}/${MAX_RECONNECT_ATTEMPTS})...`);
            setTimeout(ReconnectBLE, 2000);
        }
    }
}

async function ConnectBLE() {
    try {
        console.log('Solicitando dispositivo BLE...');
        
        // Solicitar dispositivo BLE
        bleDevice = await navigator.bluetooth.requestDevice({
            filters: [
                { namePrefix: 'PIKAM' }
            ],
            optionalServices: [SERVICE_UUID]
        });

        console.log('Dispositivo seleccionado:', bleDevice.name);
        
        // Usar la función común de conexión
        await ConnectToDevice();
        
    } catch (error) {
        console.error('Error solicitando dispositivo BLE:', error);
        EstatConn(false);
    }
}

async function ReconnectBLE() {
    if (bleDevice && bleDevice.gatt.connected) {
        console.log('Ya conectado');
        return;
    }
    
    try {
        console.log('Reconectando...');
        bleServer = await bleDevice.gatt.connect();
        
        const service = await bleServer.getPrimaryService(SERVICE_UUID);
        txCharacteristic = await service.getCharacteristic(CHARACTERISTIC_UUID_TX);
        rxCharacteristic = await service.getCharacteristic(CHARACTERISTIC_UUID_RX);
        
        await txCharacteristic.startNotifications();
        txCharacteristic.addEventListener('characteristicvaluechanged', HandleBLENotification);
        
        console.log('Reconectado exitosamente');
        EstatConn(true);
        reconnectAttempts = 0;
        EnviaDatos();
        
    } catch (error) {
        console.error('Error reconectando:', error);
        EstatConn(false);
        
        if (reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
            reconnectAttempts++;
            setTimeout(ReconnectBLE, 2000);
        }
    }
}

function OnBLEDisconnected() {
    console.log('BLE desconectado');
    EstatConn(false);
    
    // Intentar reconectar
    if (reconnectAttempts < MAX_RECONNECT_ATTEMPTS) {
        reconnectAttempts++;
        setTimeout(ReconnectBLE, 2000);
    }
}

function HandleBLENotification(event) {
    const value = event.target.value;
    const decoder = new TextDecoder('utf-8');
    const data = decoder.decode(value);
    
    // Manejar fragmentación (si el JSON viene en múltiples paquetes)
    dataBuffer += data;
    
    // Intentar procesar JSON completo
    if (dataBuffer.includes('}')) {
        try {
            // Extraer JSONs completos del buffer
            const jsonEnd = dataBuffer.lastIndexOf('}') + 1;
            const jsonStr = dataBuffer.substring(0, jsonEnd);
            
            Reset_msg_tOUT();
            Rebut(jsonStr);
            
            if (!connectat) EstatConn(true);
            
            // Limpiar buffer procesado
            dataBuffer = dataBuffer.substring(jsonEnd);
            
        } catch (error) {
            console.error('Error procesando JSON:', error);
            // Si hay error, reiniciar buffer
            dataBuffer = '';
        }
    }
}

async function SendBLE(data) {
    if (!rxCharacteristic) {
        console.error('BLE no conectado');
        return false;
    }
    
    try {
        const encoder = new TextEncoder();
        const dataArray = encoder.encode(data);
        
        // BLE tiene límite de MTU (~512 bytes), fragmentar si necesario
        const chunkSize = 512;
        for (let i = 0; i < dataArray.length; i += chunkSize) {
            const chunk = dataArray.slice(i, Math.min(i + chunkSize, dataArray.length));
            await rxCharacteristic.writeValue(chunk);
            
            // Pequeña pausa entre chunks si hay fragmentación
            if (dataArray.length > chunkSize) {
                await new Promise(resolve => setTimeout(resolve, 10));
            }
        }
        
        return true;
    } catch (error) {
        console.error('Error enviando datos BLE:', error);
        EstatConn(false);
        return false;
    }
}

function IsBLEConnected() {
    return bleDevice && bleDevice.gatt && bleDevice.gatt.connected;
}

async function DisconnectBLE() {
    if (bleDevice && bleDevice.gatt.connected) {
        await bleDevice.gatt.disconnect();
        console.log('BLE desconectado manualmente');
    }
}

function OpenFullscreen() {
    var elem = document.documentElement;
    if (elem.requestFullscreen) {
        elem.requestFullscreen();
    } else if (elem.webkitRequestFullscreen) {
        elem.webkitRequestFullscreen();
    } else if (elem.msRequestFullscreen) {
        elem.msRequestFullscreen();
    }
}

// Compatibilidad con código existente (emular interfaz WebSocket)
const ws = {
    send: function(data) {
        SendBLE(data);
    },
    readyState: {
        get OPEN() { return IsBLEConnected() ? 1 : 0; },
        get CLOSED() { return IsBLEConnected() ? 0 : 3; }
    }
};

// Alias para compatibilidad
function OpenWebsocket() {
    console.log('Intentando conexión BLE automática...');
    AutoConnectBLE();
}

// Iniciar autoconexión al cargar la página
window.addEventListener('load', () => {
    console.log('Página cargada, intentando autoconectar...');
    setTimeout(AutoConnectBLE, 1000); // Esperar 1 segundo para que todo esté listo
});