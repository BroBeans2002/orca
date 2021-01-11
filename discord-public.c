#include <stdio.h>
#include <stdlib.h>
#include <string.h> //@todo remove when settings is updated
#include <ctype.h>

#include <libdiscord.h>
#include "discord-common.h"


discord_t*
discord_init(char token[])
{
  discord_t *new_client = malloc(sizeof *new_client);
  if (NULL == new_client) return NULL;

  //trim token at non-printable character (if any)
  for (int i=0; token[i] != '\0'; ++i) {
    if (!isgraph(token[i])) {
      token[i] = '\0';
      break;
    }
  }

  Discord_api_init(&new_client->api, token);
  Discord_ws_init(&new_client->ws, token);

  /* THIS IS TEMPORARY */
  new_client->settings.token = strdup(token);
  new_client->settings.f_dump = NULL;
  /* * * * * * * * * * */

  return new_client;
}

void
discord_cleanup(discord_t *client) {
  Discord_api_cleanup(&client->api);
  Discord_ws_cleanup(&client->ws);
  if (client->settings.token)
    free(client->settings.token);
  if (client->settings.f_dump)
    fclose(client->settings.f_dump);
  free(client);
}

void
discord_global_init() {
  int code = curl_global_init(CURL_GLOBAL_DEFAULT);
  ASSERT_S(!code, "Couldn't start libcurl's globals configurations");
}

void
discord_global_cleanup() {
  curl_global_cleanup();
}

void
discord_set_on_ready(discord_t *client, discord_onrdy_cb *user_cb){
  Discord_ws_set_on_ready(&client->ws, user_cb);
}

void
discord_set_on_message(discord_t *client, discord_onmsg_cb *user_cb){
  Discord_ws_set_on_message(&client->ws, user_cb);
}

void
discord_run(discord_t *client){
  Discord_ws_run(&client->ws);
}

void
discord_dump_json(discord_t *client, char file[])
{
  FILE *f_dump = fopen(file, "a+");
  ASSERT_S(NULL != f_dump, "Could not create dump file");

  client->settings.f_dump = f_dump;  
}