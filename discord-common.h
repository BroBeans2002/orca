#ifndef LIBDISCORD_COMMON_H_
#define LIBDISCORD_COMMON_H_

//#include <libdiscord.h> (implicit)

#include <curl/curl.h>
#include "json-scanf.h"

#include "discord-tool-debug.h"

enum http_method {
  DELETE,
  GET,
  POST,
  PATCH,
  PUT,
};

/* ENDPOINTS */
#define CHANNEL           "/channels/%s"
#define CHANNEL_MESSAGES  CHANNEL"/messages"

#define GUILD             "/guilds/%s"
#define GUILD_CHANNELS    GUILD"/channels"

#define USER              "/users/%s"
#define USER_GUILDS       USER"/guilds"


enum discord_limits {
  MAX_NAME_LEN           = 100,
  MAX_TOPIC_LEN          = 1024,
  MAX_DESCRIPTION_LEN    = 1024,
  MAX_USERNAME_LEN       = 32,
  MAX_DISCRIMINATOR_LEN  = 4,
  MAX_HASH_LEN           = 1024,
  MAX_LOCALE_LEN         = 15,
  MAX_EMAIL_LEN          = 254,
  MAX_REGION_LEN         = 15,
  MAX_HEADER_LEN         = 512,
  MAX_URL_LEN            = 512,
  MAX_MESSAGE_LEN        = 2048,
  MAX_PAYLOAD_LEN        = 4096,
};

/* HTTP RESPONSE CODES
https://discord.com/developers/docs/topics/opcodes-and-status-codes#http-http-response-codes */
enum discord_http_code {
  HTTP_OK                       = 200,
  HTTP_CREATED                  = 201,
  HTTP_NO_CONTENT               = 204,
  HTTP_NOT_MODIFIED             = 304,
  HTTP_BAD_REQUEST              = 400,
  HTTP_UNAUTHORIZED             = 401,
  HTTP_FORBIDDEN                = 403,
  HTTP_NOT_FOUND                = 404,
  HTTP_METHOD_NOT_ALLOWED       = 405,
  HTTP_TOO_MANY_REQUESTS        = 429,
  HTTP_GATEWAY_UNAVAILABLE      = 502,

  CURL_NO_RESPONSE              = 0,
};

/* SNOWFLAKES
https://discord.com/developers/docs/reference#snowflakes */
enum discord_snowflake {
  SNOWFLAKE_INCREMENT           = 12,
  SNOWFLAKE_PROCESS_ID          = 17,
  SNOWFLAKE_INTERNAL_WORKER_ID  = 22,
  SNOWFLAKE_TIMESTAMP           = 64,
};

struct api_response_s {
  char *str; //the response str
  size_t size; //the response str length
};

#define MAX_HEADER_SIZE 1000

struct api_header_s {
  char *key[MAX_HEADER_SIZE];
  char *field[MAX_HEADER_SIZE];
  int size;
};

struct discord_api_s {
  CURL *ehandle; //the curl's easy handle used to perform requests
  struct curl_slist *req_header; //the request header sent to the api
  struct api_response_s res_body; //the api response string
  struct api_header_s res_pairs; //the key/field pairs response header
};

/* GATEWAY INTENTS
https://discord.com/developers/docs/topics/gateway#identify-identify-structure */
enum ws_intents {
  GUILDS                        = 1 << 0,
  GUILD_MEMBERS                 = 1 << 1,
  GUILD_BANS                    = 1 << 2,
  GUILD_EMOJIS                  = 1 << 3,
  GUILD_INTEGRATIONS            = 1 << 4,
  GUILD_WEBHOOKS                = 1 << 5,
  GUILD_INVITES                 = 1 << 6,
  GUILD_VOICE_STATES            = 1 << 7,
  GUILD_PRESENCES               = 1 << 8,
  GUILD_MESSAGES                = 1 << 9,
  GUILD_MESSAGE_REACTIONS       = 1 << 10,
  GUILD_MESSAGE_TYPING          = 1 << 11,
  DIRECT_MESSAGES               = 1 << 12,
  DIRECT_MESSAGE_REACTIONS      = 1 << 13,
  DIRECT_MESSAGE_TYPING         = 1 << 14,
};

/* GATEWAY OPCODES
https://discord.com/developers/docs/topics/opcodes-and-status-codes#gateway-gateway-opcodes */
enum ws_opcode {
  GATEWAY_DISPATCH              = 0,
  GATEWAY_HEARTBEAT             = 1,
  GATEWAY_IDENTIFY              = 2,
  GATEWAY_PRESENCE_UPDATE       = 3,
  GATEWAY_VOICE_STATE_UPDATE    = 4,
  GATEWAY_RESUME                = 6,
  GATEWAY_RECONNECT             = 7,
  GATEWAY_REQUEST_GUILD_MEMBERS = 8,
  GATEWAY_INVALID_SESSION       = 9,
  GATEWAY_HELLO                 = 10,
  GATEWAY_HEARTBEAT_ACK         = 11,
};

enum ws_status {
  WS_DISCONNECTED, //connected to ws
  WS_CONNECTED //disconnected from ws
};

struct discord_ws_s {
  enum ws_status status;
  char *identify;
  char *session_id;

  CURLM *mhandle;
  CURL *ehandle;

  struct { /* PAYLOAD STRUCTURE */
    enum ws_opcode opcode; //field 'op'
    int seq_number; //field 's'
    char event_name[16]; //field 't'
    char event_data[2048]; //field 'd'
  } payload;

  struct { /* HEARTBEAT STRUCTURE */
    long interval_ms; //interval between heartbeats
    long start_ms; //start pulse in milliseconds
  } hbeat;

  struct { /* CALLBACKS STRUCTURE */
    discord_onrdy_cb *on_ready;
    discord_onmsg_cb *on_message;
  } cbs;
};

struct _settings_s { //@todo this whole struct is temporary
  char *token;
  FILE *f_dump;
};

typedef struct discord_s {
  struct discord_ws_s ws; //discord_t == (discord_t)(ws)
  struct discord_api_s api; //discord_t == (discord_t)(api-sizeof(ws))
  struct _settings_s settings;
} discord_t;

/*for using Discord_api_request() as a template for every
 * kind of transfer*/
typedef void (discord_load_obj_cb)(void **p_obj, char *str);

/* discord-api.c */

void Discord_api_init(struct discord_api_s *api, char token[]);
void Discord_api_cleanup(struct discord_api_s *api);

void Discord_api_load_message(void **p_message, char *str);
void Discord_api_load_guild(void **p_guild, char *str);
void Discord_api_load_user(void **p_user, char *str);

void Discord_api_request(
  struct discord_api_s *api, 
  void **p_object, 
  discord_load_obj_cb *load_cb,
  char send_payload[], //only for POST/PUT methods
  enum http_method http_method,
  char endpoint[],
  ...);

/* discord-websockets.c */

void Discord_ws_init(struct discord_ws_s *ws, char token[]);
void Discord_ws_cleanup(struct discord_ws_s *ws);

void Discord_ws_set_on_ready(struct discord_ws_s *ws, discord_onrdy_cb *user_cb);
void Discord_ws_set_on_message(struct discord_ws_s *ws, discord_onmsg_cb *user_cb);

void Discord_ws_run(struct discord_ws_s *ws);

#endif