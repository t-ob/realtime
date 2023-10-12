use std::cell::RefCell;
use std::rc::Rc;

use wasm_bindgen::prelude::*;

use wasm_bindgen::{JsCast, JsValue};
use web_sys::{MessageEvent, WebSocket};
use js_sys::Uint8Array;

use serde::{Serialize, Deserialize};

#[wasm_bindgen]
pub struct Manager {
    callbacks: Rc<RefCell<Vec<js_sys::Function>>>,
}

#[wasm_bindgen]
impl Manager {
    #[wasm_bindgen(constructor)]
    pub fn new() -> Manager {
        Manager { callbacks: Rc::new(RefCell::new(Vec::new())) }
    }

    pub fn add_callback(&mut self, callback: &js_sys::Function) {
        self.callbacks.borrow_mut().push(callback.clone());
    }

    pub fn notify_callbacks(&self, message: MessageData) {
        let callbacks = self.callbacks.borrow();
        for callback in &*callbacks {
            let this = JsValue::NULL;
            let message_js = serde_wasm_bindgen::to_value(&message).unwrap();
            let _ = callback.call1(&this, &message_js);
        }
    }
}


#[wasm_bindgen]
#[derive(Serialize, Deserialize)]
pub struct MessageData {
    label: String,
    size: Vec<u32>,
    data: Vec<f32>,
}

#[wasm_bindgen]
impl MessageData {
    pub fn new(label: String, size: Vec<u32>, data: Vec<f32>) -> MessageData {
        MessageData { label, size, data }
    }

    // Additional methods for interacting with the MessageData if needed
    pub fn get_label(&self) -> String {
        self.label.clone()
    }

    pub fn get_size(&self) -> Vec<u32> {
        self.size.clone()
    }

    pub fn get_data(&self) -> Vec<f32> {
        self.data.clone()
    }
}


#[wasm_bindgen]
pub fn start_websocket(manager: Manager, url: &str) -> Result<(), JsValue> {
    let callbacks = manager.callbacks.clone();

    // Create a WebSocket connection
    let ws = WebSocket::new(url)?;
    ws.set_binary_type(web_sys::BinaryType::Arraybuffer);

    // Set up an event listener for incoming WebSocket messages
    let onmessage_callback = Closure::wrap(Box::new(move |e: MessageEvent| {
        // Log the message data to console
        if let Ok(data) = e.data().dyn_into::<js_sys::ArrayBuffer>() {
            
            let bytes = Uint8Array::new(&data);
            
            let mut header_bytes = [0u8; 12];
            bytes.slice(0, 12).copy_to(&mut header_bytes);

            let a = u32::from_le_bytes([header_bytes[0], header_bytes[1], header_bytes[2], header_bytes[3]]) as usize;
            let b = u32::from_le_bytes([header_bytes[4], header_bytes[5], header_bytes[6], header_bytes[7]]) as usize;
            let c = u32::from_le_bytes([header_bytes[8], header_bytes[9], header_bytes[10], header_bytes[11]]) as usize;

            let mut body = vec![0u8; (a + b + c) as usize];
            bytes.slice(12, 12 + (a + b + c) as u32).copy_to(&mut body);

            let label = String::from_utf8_lossy(&body[0..a]).to_string();

            let mut size = vec![0u32; b / 4];
            for i in 0..(b/4) {
                size[i] = u32::from_le_bytes([
                    body[a + 4*i], body[a + 4*i + 1], body[a + 4*i + 2], body[a + 4*i + 3]
                ]);
            }

            let mut data = vec![0f32; c / 4];
            for i in 0..(c/4) {
                let idx = a + b + 4*i;
                data[i] = f32::from_le_bytes([
                    body[idx], body[idx + 1], body[idx + 2], body[idx + 3]
                ]);
            }

            let message_data = MessageData::new(label, size, data);

            for callback in &*callbacks.borrow() {
                let this = JsValue::NULL;
                let message_js = serde_wasm_bindgen::to_value(&message_data).unwrap();
                let _ = callback.call1(&this, &message_js);
            }
        } else {
            web_sys::console::log_1(&"Received non-binary data".into());
        }
    }) as Box<dyn FnMut(MessageEvent)>);

    // Attach the event listener
    ws.set_onmessage(Some(onmessage_callback.as_ref().unchecked_ref()));
    onmessage_callback.forget(); // Leak the closure's memory, to keep it alive

    // Set up an event listener for WebSocket open event
    let onopen_callback = Closure::wrap(Box::new(move |_| {
        // Send a message when the socket is opened
        // ws_clone.send_with_str("Hello, server!").expect("Failed to send a message");
    }) as Box<dyn FnMut(JsValue)>);

    // Attach the event listener
    ws.set_onopen(Some(onopen_callback.as_ref().unchecked_ref()));
    onopen_callback.forget(); // Leak the closure's memory, to keep it alive

    Ok(())
}
