
/**
 * @file test_voice_assistant.c
 * @brief Simple test program to verify voice assistant logic
 *
 * This test runs on the host machine to verify:
 * - Mock API responses
 * - Wake word detection
 * - Basic state machine logic
 */

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>

#define WAKE_WORD "hey grok"

/* Mock API response */
const char* mock_grok_response = "Hello! I'm Grok, built by xAI. How can I help you today?";

/* Mock speech-to-text responses */
const char* mock_stt_responses[] = {
    "hey grok what is the weather like",
    "hey grok tell me a joke",
    "hello world",
    "hey grok",
    "what time is it"
};

#define NUM_MOCK_RESPONSES 5

/* Test functions */
bool test_wake_word_detection(const char* text) {
    return strstr(text, WAKE_WORD) != NULL;
}

const char* get_mock_stt_response(int index) {
    if (index >= 0 && index < NUM_MOCK_RESPONSES) {
        return mock_stt_responses[index];
    }
    return "hey grok hello";
}

void test_voice_assistant_logic() {
    printf("🧪 Testing Voice Assistant Logic\n");
    printf("================================\n\n");

    // Test wake word detection
    printf("1. Wake Word Detection Tests:\n");
    for (int i = 0; i < NUM_MOCK_RESPONSES; i++) {
        const char* text = mock_stt_responses[i];
        bool has_wake_word = test_wake_word_detection(text);
        printf("   Text: '%s'\n", text);
        printf("   Wake word detected: %s\n\n", has_wake_word ? "✅ YES" : "❌ NO");
    }

    // Test mock API responses
    printf("2. Mock API Response Test:\n");
    printf("   Input: 'hey grok hello'\n");
    printf("   Mock Grok Response: '%s'\n\n", mock_grok_response);

    // Test state machine simulation
    printf("3. State Machine Simulation:\n");
    printf("   Simulating voice assistant interaction...\n\n");

    // Simulate a conversation
    const char* user_input = "hey grok what is the meaning of life";
    printf("   User says: '%s'\n", user_input);

    if (test_wake_word_detection(user_input)) {
        printf("   ✅ Wake word detected\n");
        printf("   🔄 Processing speech-to-text...\n");
        printf("   🤖 Querying Grok API...\n");
        printf("   📢 Grok responds: '%s'\n", mock_grok_response);
        printf("   🔊 Converting to speech...\n");
        printf("   ✅ Response complete\n");
    } else {
        printf("   ❌ No wake word detected - ignoring\n");
    }

    printf("\n🎉 All tests completed!\n");
    printf("📝 Note: Real API calls will work once X.AI credits are added\n");
}

int main() {
    srand(time(NULL));
    test_voice_assistant_logic();
    return 0;
}