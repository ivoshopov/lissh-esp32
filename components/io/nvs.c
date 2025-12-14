#include <module/module.h>
#include <nvs_flash.h>
#include <io.h>
#include <module/io/codec/text.h>

#define STORAGE_NAMESPACE "storage"

struct nvs_typ {
  char *data;
  size_t pos;
  size_t size;
};

struct nvs_typ nvs_init_script = {
  .data = NULL,
  .pos = 0,
  .size = 0,
};

static char nil_buff[] = "()";

/* Read a char from an NVS entry */
static int nvs_read(struct io_primitive *buff) {
  struct nvs_typ *nvs = buff->private;
  assert(nvs != NULL);
  if (nvs->data == NULL) {
    /* Allocate memory and fill it */
    nvs_handle_t handle;
    esp_err_t err;

    nvs->pos = 0;
    err = nvs_open(STORAGE_NAMESPACE, NVS_READONLY, &handle);
    if (err == ESP_OK) {
      err = nvs_get_blob(handle, "init-script", NULL, &nvs->size);
      if (err == ESP_OK) {
        nvs->data = malloc(nvs->size);
        err = nvs_get_blob(handle, "init-script", nvs->data, &nvs->size);
        if (err != ESP_OK) {
          free(nvs->data);
          return EOF;
        }
      } else if (err == ESP_ERR_NVS_NOT_FOUND) {
        /* When the flash memory is empty we should replay with correct error
         * message. Since there it isn't possible right now we can just return
         * "()". It isn't perfect solution since nvs layer shouldn't relay on
         * text based encoding, some day we can have binary encoding of lisp
         * expresion. */
        nvs->data = nil_buff;
      }
      nvs_close(handle);
    } else {
      return EOF;
    }
  }
  if (nvs->pos == nvs->size)
    return EOF;
  char c = nvs->data[nvs->pos];
  nvs->pos++;
  /* When we reach the end of the string in the file we reset the pos pointer */
  if (c == '\0' || c == '\n')
    nvs->pos = 0;
  return c;
}


/* Write a char to a file */
static int nvs_write(struct io_primitive *buff, char c) {
  struct nvs_typ *nvs = buff->private;
  assert(nvs != NULL);
  if (nvs->data == NULL) {
    /* Inital memory allocation */
    nvs->size = 256;
    nvs->data = malloc(nvs->size);
    if (nvs->data == NULL) {
      printf("Not enough dynamic memory\n");
      return EOF;
    }
  }
  if (nvs->pos == nvs->size) {
    nvs->size = nvs->size * 2;
    nvs->data = realloc(nvs->data, nvs->size);
    if (nvs->data == NULL) {
      printf("Not enough dynamic memory\n");
      return EOF;
    }
  }
  nvs->data[nvs->pos] = c;
  nvs->pos++;
  /* When we reach the end of the string we reset the pos pointer */
  if (c == '\0' || c == '\n') {
    nvs->pos = 0;
    nvs_handle_t handle;
    esp_err_t err;

    err = nvs_open(STORAGE_NAMESPACE, NVS_READWRITE, &handle);
    if (err != ESP_OK) return EOF;

    err = nvs_set_blob(handle, "init-script", nvs->data, nvs->size);
    if (err != ESP_OK) return EOF;

    err = nvs_commit(handle);
    if (err != ESP_OK) return EOF;

    nvs_close(handle);
  }
  return 0;
}


static struct io_primitive init_script_io = {
  .read = nvs_read,
  .write = nvs_write,
  .private = (void*)&nvs_init_script,
};

PORTS_SECTION struct io_typ init_script_port = {
  .private = &init_script_io,
  .read = text_read,
  .write = text_write,
  .proto = "init-script.lisp",
};


static void init() {
  // Initialize NVS
  esp_err_t err = nvs_flash_init();
  if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    err = nvs_flash_init();
  }
  ESP_ERROR_CHECK(err);
}

MODULE_SECTION struct module io_nvs_module = {
  .setup = init,
};
