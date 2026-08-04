# mulle-http Library Documentation for AI
<!-- Keywords: http, parsing -->

## 1. Introduction & Purpose

**mulle-http** is a high-performance HTTP/1.1 request and response parser extracted from the NGINX codebase (originally from NODE.js). It provides:

- **Fast, standard-compliant HTTP parsing**: Based on proven NGINX HTTP parser
- **Callback-driven architecture**: Incremental parsing with callbacks for different message components
- **Request and response parsing**: Handles both HTTP request parsing and response parsing
- **Chunked transfer encoding support**: Understands chunked request/response bodies
- **URL field extraction**: Parses and separates URL components (scheme, host, port, path, query, fragment)
- **Header parsing**: Incremental header field and value callbacks
- **Upgrade header detection**: Recognizes protocol upgrades (e.g., for WebSocket)
- **Strict and lenient modes**: Configurable strict HTTP compliance

This library is a foundational component of mulle-core and provides the basis for HTTP processing in web frameworks and network libraries.

## 2. Key Concepts & Design Philosophy

- **Incremental parsing**: Parser does not require entire message in memory; feeds data in chunks
- **Callback-driven**: Uses callbacks for each major HTTP component (URL, headers, body, etc.) rather than returning structured data
- **Zero-copy design**: Callbacks provide pointers and lengths to data rather than copying strings
- **Stateful**: Parser maintains state machine internally; clients provide data via `http_parser_execute`
- **Dual-mode**: Can parse HTTP requests or responses (or both, with automatic detection)
- **Non-blocking**: Suitable for event-driven, non-blocking I/O servers
- **Strict vs. lenient**: `HTTP_PARSER_STRICT` mode enforces full RFC compliance; lenient mode is more forgiving

## 3. Core API & Data Structures

### 3.1. `http_parser.h`

#### `struct http_parser`
- **Purpose**: Maintains parser state for incremental HTTP message parsing
- **Key Public Fields** (READ-ONLY after parsing begins):
  - `unsigned short http_major`: Major version (e.g., 1 for HTTP/1.1)
  - `unsigned short http_minor`: Minor version (e.g., 1 for HTTP/1.1)
  - `unsigned int status_code`: Response status code (e.g., 200, 404) - only for responses
  - `unsigned int method`: Request method enum (e.g., HTTP_GET, HTTP_POST) - only for requests
  - `unsigned int upgrade`: 1 if Upgrade header present and connection should upgrade, 0 otherwise
  - `uint64_t content_length`: Content-Length header value (0 if absent); also chunk length during chunk parsing

- **Key Public Fields** (WRITABLE):
  - `void *data`: Opaque pointer for user context (often points to connection/socket object)

- **Private Fields**: State machine internals, flags, counters (do not access directly)

#### Parser Type Enum
```c
enum http_parser_type {
  HTTP_REQUEST,   // Parse HTTP requests
  HTTP_RESPONSE,  // Parse HTTP responses
  HTTP_BOTH       // Auto-detect request or response
};
```

#### HTTP Methods Enum
```c
enum http_method {
  HTTP_DELETE, HTTP_GET, HTTP_HEAD, HTTP_POST, HTTP_PUT,
  HTTP_CONNECT, HTTP_OPTIONS, HTTP_TRACE, HTTP_COPY,
  HTTP_LOCK, HTTP_MKCOL, HTTP_MOVE, HTTP_PROPFIND,
  HTTP_PROPPATCH, HTTP_SEARCH, HTTP_UNLOCK, HTTP_BIND,
  HTTP_REBIND, HTTP_UNBIND, HTTP_ACL, HTTP_REPORT,
  HTTP_MKACTIVITY, HTTP_CHECKOUT, HTTP_MERGE, HTTP_MSEARCH,
  HTTP_NOTIFY, HTTP_SUBSCRIBE, HTTP_UNSUBSCRIBE, HTTP_PATCH
};
```

#### Parser Callbacks

**Callback Type Definitions:**
- `typedef int (*http_data_cb)(http_parser *parser, const char *at, size_t length)`: Called for data chunks (URL, headers, body)
- `typedef int (*http_cb)(http_parser *parser)`: Called for events without data (headers complete, message complete)

**Callback Return Values:**
- Return 0 to continue parsing
- Return non-zero to signal parse error and stop
- Special: `on_headers_complete` can return:
  - 0: Continue parsing, expect body
  - 1: (Response only) Don't expect body (HEAD response or similar)
  - 2: (Response only) No further responses expected on connection (CONNECT response)

#### `struct http_parser_settings`
- **Purpose**: Callback handlers for different parsing events
- **Key Callbacks**:
  - `http_cb on_message_begin`: Called when message parsing starts
  - `http_data_cb on_url`: Called with URL data (may be called multiple times for long URLs)
  - `http_data_cb on_status`: Called with status text (response only)
  - `http_data_cb on_header_field`: Called with header field name
  - `http_data_cb on_header_value`: Called with header field value
  - `http_cb on_headers_complete`: Called after all headers parsed (before body)
  - `http_data_cb on_body`: Called with body data (may be multiple times for large bodies)
  - `http_cb on_message_complete`: Called after entire message parsed
  - `http_cb on_chunk_header`: Called when chunk header parsed (chunked encoding)
  - `http_cb on_chunk_complete`: Called after chunk body parsed

#### URL Parsing Fields Enum
```c
enum http_parser_url_fields {
  UF_SCHEMA,          // http, https, etc.
  UF_HOST,            // hostname or IP
  UF_PORT,            // numeric port number
  UF_PATH,            // /path/to/resource
  UF_QUERY,           // ?query=string
  UF_FRAGMENT,        // #anchor
  UF_USERINFO         // user:password (in authority)
};
```

#### `struct http_parser_url`
- **Purpose**: Result of URL parsing (extract URL components)
- **Key Fields**:
  - `uint16_t field_set`: Bitmask indicating which fields are present
  - `uint16_t port`: Parsed port number (0 if not present or default)
  - `struct { uint16_t off; uint16_t len; } fields[7]`: Offsets and lengths for each URL component in original buffer

### 3.2. Core Functions

#### Initialization
- `void http_parser_init(struct http_parser *parser, enum http_parser_type type)`: Initialize parser for requests or responses
- `size_t http_parser_execute(struct http_parser *parser, const struct http_parser_settings *settings, const char *data, size_t len)`: Parse data chunk
  - Returns: Number of bytes parsed (may be less than input if error)
  - Check `parser->http_errno` for error code if `< len`

#### URL Parsing
- `int http_parser_parse_url(const char *buf, size_t buflen, int is_connect, struct http_parser_url *u)`: Parse URL into components
  - `is_connect`: 1 if parsing CONNECT request URL (authority form), 0 for normal URLs
  - Returns: 0 on success, non-zero error code on failure
  - After success, use `u->fields[UF_*].off` and `.len` to extract components from buffer

#### Version Query
- `unsigned long http_parser_version(void)`: Get version as 32-bit packed value (major, minor, patch)

#### Error Reporting
- `const char *http_errno_name(enum http_errno err)`: Get error name string
- `const char *http_errno_description(enum http_errno err)`: Get error description string
- Check `parser->http_errno` field for error code after `http_parser_execute` returns < input length

### 3.3. Configuration Constants

- `HTTP_PARSER_STRICT`: Compile-time flag (default 1); set to 0 for lenient parsing
- `HTTP_MAX_HEADER_SIZE`: Maximum header size allowed (default 80KB)

## 4. Performance Characteristics

- **Parsing speed**: O(n) for n bytes of input; designed for high throughput
- **Memory overhead**: Parser struct is ~100 bytes; no internal buffering (streaming)
- **Callback overhead**: Callbacks may be invoked many times for large messages (e.g., many small header chunks)
- **URL parsing**: O(n) for URL length; separate operation after message parsing
- **Thread-safety**: Parser instance is not thread-safe; each thread should have its own parser
- **Streaming**: Suitable for processing multi-gigabyte messages chunk-by-chunk

## 5. AI Usage Recommendations & Patterns

### Best Practices

1. **One parser per connection/stream**: Create separate parser instances for each HTTP connection
2. **Initialize with correct type**: Use `HTTP_REQUEST` for server, `HTTP_RESPONSE` for client
3. **Check return values**: Always verify `http_parser_execute` return value; if < input length, check `http_errno`
4. **Handle partial data**: Parser is designed for incremental input; feed data as it arrives
5. **Capture data in callbacks**: Callbacks provide pointers to data in the input buffer; copy if needed for later use
6. **URL parsing after message**: Call `http_parser_parse_url` with full URL from `on_url` callback data
7. **Check upgrade flag**: After parsing request/response, check `upgrade` field for protocol upgrades

### Common Pitfalls

1. **Buffer lifetime**: Callback data pointers (`at`) point into input buffer; copy if storing beyond callback scope
2. **Multiple message parsing**: Create new parser for each HTTP message (or reset state carefully)
3. **Error checking**: Easy to forget checking `http_parser_execute` return value; always verify
4. **Header fragmentation**: Header field names/values may arrive in multiple callbacks; accumulate with string building
5. **HTTP/1.1 pipelining**: Parser handles one message at a time; manage pipelining at higher level
6. **Chunked encoding**: Parser handles chunked transfer encoding transparently; use `on_chunk_*` callbacks if needed
7. **Connection upgrade**: Check `upgrade` field after parsing to determine if connection should upgrade protocols

## 6. Integration Examples

### Example 1: Simple HTTP request parsing

```c
#include <mulle-http/mulle-http.h>
#include <stdio.h>
#include <string.h>

static int on_message_begin(http_parser *parser) {
    printf("Message begin\n");
    return 0;
}

static int on_url(http_parser *parser, const char *at, size_t length) {
    printf("URL: %.*s\n", (int)length, at);
    return 0;
}

static int on_header_field(http_parser *parser, const char *at, size_t length) {
    printf("Header: %.*s", (int)length, at);
    return 0;
}

static int on_header_value(http_parser *parser, const char *at, size_t length) {
    printf(" = %.*s\n", (int)length, at);
    return 0;
}

static int on_headers_complete(http_parser *parser) {
    printf("Headers complete, method=%d\n", parser->method);
    return 0;
}

static int on_body(http_parser *parser, const char *at, size_t length) {
    printf("Body: %.*s\n", (int)length, at);
    return 0;
}

static int on_message_complete(http_parser *parser) {
    printf("Message complete\n");
    return 0;
}

int main() {
    struct http_parser parser;
    struct http_parser_settings settings = {
        .on_message_begin = on_message_begin,
        .on_url = on_url,
        .on_header_field = on_header_field,
        .on_header_value = on_header_value,
        .on_headers_complete = on_headers_complete,
        .on_body = on_body,
        .on_message_complete = on_message_complete
    };
    
    http_parser_init(&parser, HTTP_REQUEST);
    
    const char *request = "GET /path HTTP/1.1\r\nHost: example.com\r\n\r\n";
    size_t parsed = http_parser_execute(&parser, &settings, request, strlen(request));
    
    printf("Parsed %zu bytes\n", parsed);
    return 0;
}
```

### Example 2: Parse URL components

```c
#include <mulle-http/mulle-http.h>
#include <stdio.h>
#include <string.h>

int main() {
    const char *url = "https://user:pass@example.com:8080/path?query=value#anchor";
    struct http_parser_url parsed;
    
    int result = http_parser_parse_url(url, strlen(url), 0, &parsed);
    
    if (result == 0) {
        printf("Schema: %.*s\n", 
            parsed.fields[UF_SCHEMA].len, 
            url + parsed.fields[UF_SCHEMA].off);
        printf("Host: %.*s\n", 
            parsed.fields[UF_HOST].len, 
            url + parsed.fields[UF_HOST].off);
        printf("Port: %d\n", parsed.port);
        printf("Path: %.*s\n", 
            parsed.fields[UF_PATH].len, 
            url + parsed.fields[UF_PATH].off);
    }
    return 0;
}
```

### Example 3: HTTP response parsing

```c
#include <mulle-http/mulle-http.h>
#include <stdio.h>
#include <string.h>

static int on_status(http_parser *parser, const char *at, size_t length) {
    printf("Status text: %.*s\n", (int)length, at);
    return 0;
}

static int on_headers_complete(http_parser *parser) {
    printf("Response status code: %d (HTTP/%d.%d)\n", 
        parser->status_code, parser->http_major, parser->http_minor);
    return 0;
}

int main() {
    struct http_parser parser;
    struct http_parser_settings settings = {
        .on_status = on_status,
        .on_headers_complete = on_headers_complete
    };
    
    http_parser_init(&parser, HTTP_RESPONSE);
    
    const char *response = "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    http_parser_execute(&parser, &settings, response, strlen(response));
    
    return 0;
}
```

### Example 4: Incremental parsing with chunks

```c
#include <mulle-http/mulle-http.h>
#include <stdio.h>
#include <string.h>

int main() {
    struct http_parser parser;
    struct http_parser_settings settings = {0};
    
    http_parser_init(&parser, HTTP_REQUEST);
    
    // Simulate receiving data in chunks
    const char *chunk1 = "GET /path HTTP/1.1\r\n";
    const char *chunk2 = "Host: example.com\r\n";
    const char *chunk3 = "Content-Length: 5\r\n\r\nhello";
    
    size_t nparsed = 0;
    
    nparsed += http_parser_execute(&parser, &settings, chunk1, strlen(chunk1));
    nparsed += http_parser_execute(&parser, &settings, chunk2, strlen(chunk2));
    nparsed += http_parser_execute(&parser, &settings, chunk3, strlen(chunk3));
    
    printf("Total parsed: %zu bytes\n", nparsed);
    printf("Error: %s\n", http_errno_name(parser.http_errno));
    
    return 0;
}
```

### Example 5: Detect upgrade requests (WebSocket, etc.)

```c
#include <mulle-http/mulle-http.h>
#include <stdio.h>

int main() {
    struct http_parser parser;
    struct http_parser_settings settings = {0};
    
    http_parser_init(&parser, HTTP_REQUEST);
    
    // WebSocket upgrade request
    const char *request = "GET /chat HTTP/1.1\r\n"
                         "Upgrade: websocket\r\n"
                         "Connection: Upgrade\r\n"
                         "\r\n";
    
    http_parser_execute(&parser, &settings, request, strlen(request));
    
    if (parser.upgrade) {
        printf("Upgrade requested (connection should switch protocols)\n");
    }
    
    return 0;
}
```

### Example 6: Accumulate fragmented headers

```c
#include <mulle-http/mulle-http.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    char current_header_field[256];
    char current_header_value[1024];
} ParseContext;

static int on_header_field(http_parser *parser, const char *at, size_t length) {
    ParseContext *ctx = (ParseContext *)parser->data;
    strncat(ctx->current_header_field, at, length);
    return 0;
}

static int on_header_value(http_parser *parser, const char *at, size_t length) {
    ParseContext *ctx = (ParseContext *)parser->data;
    strncat(ctx->current_header_value, at, length);
    return 0;
}

int main() {
    struct http_parser parser;
    struct http_parser_settings settings = {
        .on_header_field = on_header_field,
        .on_header_value = on_header_value
    };
    
    ParseContext ctx = {0};
    http_parser_init(&parser, HTTP_REQUEST);
    parser.data = &ctx;
    
    const char *request = "GET / HTTP/1.1\r\nX-Custom-Header: value\r\n\r\n";
    http_parser_execute(&parser, &settings, request, strlen(request));
    
    printf("Field: %s\n", ctx.current_header_field);
    printf("Value: %s\n", ctx.current_header_value);
    
    return 0;
}
```

## 7. Dependencies

- `mulle-c11`: Cross-platform C compiler glue

