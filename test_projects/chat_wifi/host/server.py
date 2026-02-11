#!/usr/bin/env python3
"""
Jarvis Voice Assistant - Host Server
Receives audio from ESP32-P4, processes with Whisper STT, queries Grok, returns TTS audio.
"""

import asyncio
import json
import os
import io
import logging
from datetime import datetime

import numpy as np
import websockets
from dotenv import load_dotenv
from openai import OpenAI

# Load environment variables
load_dotenv()

# Configure logging
logging.basicConfig(
    level=logging.INFO,
    format='%(asctime)s [%(levelname)s] %(message)s'
)
logger = logging.getLogger(__name__)

# Configuration
GROK_API_KEY = os.getenv('GROK_API_KEY', '')
GROK_MODEL = os.getenv('GROK_MODEL', 'grok-2-latest')
HOST = os.getenv('HOST', '0.0.0.0')
PORT = int(os.getenv('PORT', 8765))
SAMPLE_RATE = int(os.getenv('SAMPLE_RATE', 16000))

# Initialize Grok client (uses OpenAI-compatible API)
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
        logger.info(f"Loading Whisper model: {model_name}")
        whisper_model = whisper.load_model(model_name)
    return whisper_model

def transcribe_audio(audio_data: bytes) -> str:
    """Transcribe audio bytes to text using Whisper"""
    model = load_whisper()
    
    # Convert bytes to numpy array (assuming 16-bit PCM)
    audio_np = np.frombuffer(audio_data, dtype=np.int16).astype(np.float32) / 32768.0
    
    # Transcribe
    result = model.transcribe(audio_np, fp16=False)
    text = result['text'].strip()
    
    logger.info(f"Transcribed: {text}")
    return text

async def query_grok(user_message: str, conversation_history: list) -> str:
    """Query Grok API with conversation context"""
    
    if not GROK_API_KEY:
        return "Error: Grok API key not configured. Please set GROK_API_KEY in .env file."
    
    # Add user message to history
    conversation_history.append({
        "role": "user",
        "content": user_message
    })
    
    try:
        # System prompt for Jarvis personality
        messages = [
            {
                "role": "system",
                "content": """You are Jarvis, a helpful AI assistant. 
                Keep responses concise and conversational since they will be spoken aloud.
                Be friendly, helpful, and occasionally witty."""
            }
        ] + conversation_history[-10:]  # Keep last 10 messages for context
        
        response = grok_client.chat.completions.create(
            model=GROK_MODEL,
            messages=messages,
            max_tokens=500,
            temperature=0.7
        )
        
        assistant_message = response.choices[0].message.content
        
        # Add to history
        conversation_history.append({
            "role": "assistant",
            "content": assistant_message
        })
        
        logger.info(f"Grok response: {assistant_message[:100]}...")
        return assistant_message
        
    except Exception as e:
        logger.error(f"Grok API error: {e}")
        return f"Sorry, I encountered an error: {str(e)}"

def text_to_speech(text: str) -> bytes:
    """Convert text to speech audio bytes"""
    # TODO: Implement with Piper or other TTS
    # For now, return empty (ESP32 will just show text)
    logger.info(f"TTS (not implemented): {text[:50]}...")
    return b''

class ClientSession:
    """Manages state for a connected ESP32 client"""
    
    def __init__(self, websocket):
        self.websocket = websocket
        self.conversation_history = []
        self.audio_buffer = bytearray()
        self.is_recording = False
        
    async def handle_message(self, message):
        """Handle incoming WebSocket message"""
        
        if isinstance(message, bytes):
            # Audio data
            if self.is_recording:
                self.audio_buffer.extend(message)
                
        else:
            # JSON command
            try:
                cmd = json.loads(message)
                await self.handle_command(cmd)
            except json.JSONDecodeError:
                logger.warning(f"Invalid JSON: {message}")
    
    async def handle_command(self, cmd):
        """Handle JSON command from ESP32"""
        
        action = cmd.get('action')
        
        if action == 'start_recording':
            self.audio_buffer.clear()
            self.is_recording = True
            logger.info("Recording started")
            await self.send_response({'status': 'recording'})
            
        elif action == 'stop_recording':
            self.is_recording = False
            logger.info(f"Recording stopped, {len(self.audio_buffer)} bytes")
            
            if len(self.audio_buffer) > 0:
                # Process the audio
                await self.process_audio()
            else:
                await self.send_response({'status': 'error', 'message': 'No audio data'})
                
        elif action == 'ping':
            await self.send_response({'status': 'pong'})
            
        elif action == 'clear_history':
            self.conversation_history.clear()
            await self.send_response({'status': 'history_cleared'})
    
    async def process_audio(self):
        """Process recorded audio: STT -> Grok -> TTS"""
        
        try:
            # Step 1: Speech to Text
            await self.send_response({'status': 'transcribing'})
            text = transcribe_audio(bytes(self.audio_buffer))
            
            if not text:
                await self.send_response({'status': 'error', 'message': 'Could not transcribe audio'})
                return
            
            await self.send_response({'status': 'transcribed', 'text': text})
            
            # Step 2: Query Grok
            await self.send_response({'status': 'thinking'})
            response = await query_grok(text, self.conversation_history)
            
            await self.send_response({
                'status': 'response',
                'user_text': text,
                'assistant_text': response
            })
            
            # Step 3: Text to Speech (optional)
            # audio = text_to_speech(response)
            # if audio:
            #     await self.websocket.send(audio)
            
        except Exception as e:
            logger.error(f"Processing error: {e}")
            await self.send_response({'status': 'error', 'message': str(e)})
    
    async def send_response(self, data):
        """Send JSON response to ESP32"""
        await self.websocket.send(json.dumps(data))

async def handle_client(websocket, path=None):
    """Handle WebSocket client connection"""
    
    client_ip = websocket.remote_address[0]
    logger.info(f"Client connected: {client_ip}")
    
    session = ClientSession(websocket)
    
    try:
        async for message in websocket:
            await session.handle_message(message)
            
    except websockets.exceptions.ConnectionClosed:
        logger.info(f"Client disconnected: {client_ip}")
    except Exception as e:
        logger.error(f"Client error: {e}")

async def main():
    """Start WebSocket server"""
    
    logger.info(f"Starting Jarvis server on ws://{HOST}:{PORT}")
    
    if not GROK_API_KEY:
        logger.warning("GROK_API_KEY not set! Chat will not work.")
    
    async with websockets.serve(handle_client, HOST, PORT):
        logger.info("Server ready. Waiting for ESP32 connections...")
        await asyncio.Future()  # Run forever

if __name__ == '__main__':
    asyncio.run(main())
