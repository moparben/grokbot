#!/usr/bin/env python3
"""
Jarvis Voice Assistant - Host Server (USB Serial Mode)
Receives audio from ESP32-P4 over USB serial, processes with Whisper STT, queries Grok.

Usage:
    python server_serial.py [--port /dev/ttyACM0] [--baud 115200]
"""

import argparse
import json
import os
import sys
import struct
import time
import threading
import wave
import tempfile
from datetime import datetime

import serial
import numpy as np
from dotenv import load_dotenv
from openai import OpenAI

# Load environment variables
load_dotenv()

# Configuration
GROK_API_KEY = os.getenv('GROK_API_KEY', '')
GROK_MODEL = os.getenv('GROK_MODEL', 'grok-2-latest')
SAMPLE_RATE = 16000

# Initialize Grok client
grok_client = None
if GROK_API_KEY:
    grok_client = OpenAI(
        api_key=GROK_API_KEY,
        base_url="https://api.x.ai/v1"
    )

# Whisper model (lazy loaded)
whisper_model = None

def load_whisper():
    """Lazy load Whisper model"""
    global whisper_model
    if whisper_model is None:
        import whisper
        model_name = os.getenv('WHISPER_MODEL', 'base.en')
        print(f"Loading Whisper model: {model_name}...")
        whisper_model = whisper.load_model(model_name)
        print("Whisper loaded!")
    return whisper_model

def transcribe_audio(audio_data: np.ndarray) -> str:
    """Transcribe audio array to text using Whisper"""
    model = load_whisper()
    
    # Normalize to float32
    audio_float = audio_data.astype(np.float32) / 32768.0
    
    # Transcribe
    result = model.transcribe(audio_float, fp16=False)
    text = result['text'].strip()
    
    return text

def query_grok(user_message: str, conversation_history: list) -> str:
    """Query Grok API with conversation context"""
    
    if not grok_client:
        return "Error: Grok API key not configured. Please set GROK_API_KEY in .env file."
    
    conversation_history.append({
        "role": "user",
        "content": user_message
    })
    
    try:
        messages = [
            {
                "role": "system",
                "content": """You are Jarvis, a helpful AI assistant. 
                Keep responses concise and conversational since they will be spoken aloud.
                Be friendly, helpful, and occasionally witty."""
            }
        ] + conversation_history[-10:]
        
        response = grok_client.chat.completions.create(
            model=GROK_MODEL,
            messages=messages,
            max_tokens=500,
            temperature=0.7
        )
        
        assistant_message = response.choices[0].message.content
        
        conversation_history.append({
            "role": "assistant",
            "content": assistant_message
        })
        
        return assistant_message
        
    except Exception as e:
        return f"Sorry, I encountered an error: {str(e)}"

class JarvisSerial:
    """Handle serial communication with ESP32-P4"""
    
    HEADER_MAGIC = b'JAUD'
    HEADER_SIZE = 16  # 4 bytes magic + 4 bytes seq + 4 bytes samples + 4 bytes timestamp
    
    def __init__(self, port: str, baud: int = 115200):
        self.port = port
        self.baud = baud
        self.serial = None
        self.running = False
        self.audio_buffer = []
        self.conversation_history = []
        self.recording = False
        
    def connect(self):
        """Connect to serial port"""
        print(f"Connecting to {self.port} at {self.baud} baud...")
        self.serial = serial.Serial(self.port, self.baud, timeout=0.1)
        time.sleep(2)  # Wait for ESP32 to boot
        self.serial.reset_input_buffer()
        print("Connected!")
        
    def read_line(self) -> str:
        """Read a line of text (JSON events)"""
        try:
            line = self.serial.readline().decode('utf-8', errors='ignore').strip()
            return line if line else None
        except:
            return None
    
    def read_audio_packet(self) -> tuple:
        """Read audio packet (header + data)"""
        # Look for magic bytes
        while True:
            byte = self.serial.read(1)
            if not byte:
                return None, None
            
            if byte == b'J':
                rest = self.serial.read(3)
                if rest == b'AUD':
                    break
                    
        # Read rest of header (12 bytes: seq, samples, timestamp)
        header_data = self.serial.read(12)
        if len(header_data) < 12:
            return None, None
            
        seq, samples, timestamp = struct.unpack('<III', header_data)
        
        # Read audio data
        audio_bytes = self.serial.read(samples * 2)
        if len(audio_bytes) < samples * 2:
            return None, None
            
        audio_data = np.frombuffer(audio_bytes, dtype=np.int16)
        
        return (seq, samples, timestamp), audio_data
    
    def process_recording(self):
        """Process recorded audio: STT -> Grok"""
        if not self.audio_buffer:
            print("No audio recorded!")
            return
            
        # Combine all audio packets
        audio = np.concatenate(self.audio_buffer)
        print(f"Processing {len(audio)} samples ({len(audio)/SAMPLE_RATE:.1f}s)...")
        
        # Transcribe
        print("Transcribing...")
        text = transcribe_audio(audio)
        
        if not text:
            print("Could not transcribe audio")
            return
            
        print(f"\n[You]: {text}")
        
        # Query Grok
        print("Asking Grok...")
        response = query_grok(text, self.conversation_history)
        
        print(f"[Jarvis]: {response}\n")
    
    def run(self):
        """Main loop"""
        self.connect()
        self.running = True
        
        print("\n" + "="*50)
        print("  Jarvis Voice Assistant")
        print("  Press BOOT button on ESP32 to talk")
        print("="*50 + "\n")
        
        while self.running:
            try:
                # Check for text line (JSON events)
                if self.serial.in_waiting:
                    # Peek at first byte
                    first = self.serial.read(1)
                    
                    if first == b'{':
                        # JSON event
                        rest = self.serial.readline().decode('utf-8', errors='ignore')
                        line = '{' + rest.strip()
                        
                        try:
                            event = json.loads(line)
                            self.handle_event(event)
                        except json.JSONDecodeError:
                            pass
                            
                    elif first == b'J':
                        # Possible audio packet
                        rest = self.serial.read(3)
                        if rest == b'AUD' and self.recording:
                            # Read rest of header
                            header_data = self.serial.read(12)
                            if len(header_data) == 12:
                                seq, samples, timestamp = struct.unpack('<III', header_data)
                                audio_bytes = self.serial.read(samples * 2)
                                if len(audio_bytes) == samples * 2:
                                    audio_data = np.frombuffer(audio_bytes, dtype=np.int16)
                                    self.audio_buffer.append(audio_data)
                                    print(f"\rRecording... {len(self.audio_buffer)} packets", end='', flush=True)
                    else:
                        # Some other output (ESP_LOG, etc)
                        rest = self.serial.readline()
                        
                else:
                    time.sleep(0.01)
                    
            except KeyboardInterrupt:
                print("\nExiting...")
                break
            except Exception as e:
                print(f"Error: {e}")
                time.sleep(0.1)
        
        if self.serial:
            self.serial.close()
    
    def handle_event(self, event: dict):
        """Handle JSON event from ESP32"""
        event_type = event.get('event')
        
        if event_type == 'ready':
            print(f"ESP32: {event.get('message', 'Ready')}")
            
        elif event_type == 'recording_start':
            print("\n>> Recording started...")
            self.recording = True
            self.audio_buffer = []
            
        elif event_type == 'recording_stop':
            print(f"\n>> Recording stopped ({event.get('packets', 0)} packets)")
            self.recording = False
            self.process_recording()
            
        elif event_type == 'status':
            level = event.get('level_db', -100)
            # Only show if interesting
            if level > -50:
                print(f"[Status] Level: {level:.1f} dB")

def main():
    parser = argparse.ArgumentParser(description='Jarvis Voice Assistant - Serial Mode')
    parser.add_argument('--port', '-p', default='/dev/ttyACM0',
                       help='Serial port (default: /dev/ttyACM0)')
    parser.add_argument('--baud', '-b', type=int, default=115200,
                       help='Baud rate (default: 115200)')
    
    args = parser.parse_args()
    
    # Check for Grok API key
    if not GROK_API_KEY:
        print("WARNING: GROK_API_KEY not set in .env file!")
        print("Chat responses will not work.\n")
    
    # Pre-load Whisper
    print("Initializing...")
    load_whisper()
    
    # Start
    jarvis = JarvisSerial(args.port, args.baud)
    jarvis.run()

if __name__ == '__main__':
    main()
