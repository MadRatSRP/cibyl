#include <string.h>
#include <stdlib.h>

#include <cibyl-fileops.h>
#include <cibyl-memoryfs.h>

#define min(x,y) ( (x) < (y) ? (x) : (y) )
#define max(x,y) ( (x) > (y) ? (x) : (y) )

static int seek(FILE *fp, long offset, int whence)
{
  NOPH_Memory_file_t *p = (NOPH_Memory_file_t*)fp->priv;
  int old = p->fp;

  switch (whence)
    {
    case SEEK_SET:
      p->fp = offset;
      break;
    case SEEK_CUR:
      p->fp = p->fp + offset;
      break;
    case SEEK_END:
      p->fp = p->data_size + offset;
      break;
    default:
      return -1;
    }

  if (p->fp < 0)
    {
      p->fp = old;
      return -1;
    }

  if (p->fp >= p->data_size)
    p->fp = p->data_size;

  return 0;
}

static long tell(FILE *fp)
{
  NOPH_Memory_file_t *p = (NOPH_Memory_file_t*)fp->priv;
  return p->fp;
}

static size_t read(FILE *fp, void *ptr, size_t in_size)
{
  NOPH_Memory_file_t *p = (NOPH_Memory_file_t*)fp->priv;
  size_t size = min(in_size, p->data_size - p->fp);

  if (size >= 0)
    memcpy( ptr, p->data + p->fp, size );
  p->fp = p->fp + size;

  /* End of file reached? */
  if (p->fp >= p->data_size)
    fp->eof = 1;

  return size;
}

static size_t write(FILE *fp, const void *ptr, size_t in_size)
{
  NOPH_Memory_file_t *p = (NOPH_Memory_file_t*)fp->priv;
  size_t new_size = max(p->fp + in_size, p->data_size);
  long old = p->fp;

  /* Extend the file size? */
  if (new_size > p->allocated_size)
    {
      p->data = realloc(p->data, new_size + 4096);
      if (!p->data)
        return -1;
      p->data_size = new_size;
      p->allocated_size = new_size + 4096;
      fp->eof = 0;
    }
  if (new_size > p->data_size)
    p->data_size = new_size;

  if (in_size >= 0)
    memcpy( p->data + p->fp, ptr, in_size );
  p->fp = p->fp + in_size;

  return p->fp - old;
}

static int close(FILE *fp)
{
  NOPH_Memory_file_t *p = (NOPH_Memory_file_t*)fp->priv;

  /* Writeback the file */
  if (p->writeback_path)
    {
      FILE *tmp = fopen(p->writeback_path, p->mode);

      free((void*)p->mode);
      free((void*)p->writeback_path);
      if (!tmp)
        return -1;

      if (fwrite(p->data, p->data_size, 1, tmp) != p->data_size)
        return -1;
      fclose(tmp);
    }
  /* If this is allocated data, free it on closing */
  if (p->allocate)
    free(p->data);

  return 0;
}

cibyl_fops_t NOPH_Memory_fops =
{
  .priv_data_size = sizeof(NOPH_Memory_file_t),
  .open = NULL, /* Not applicable */
  .close = close,
  .read = read,
  .write = write,
  .seek = seek,
  .tell = tell,
};

void *NOPH_MemoryFile_getDataPtr(FILE *fp)
{
  NOPH_Memory_file_t *p = (NOPH_Memory_file_t*)fp->priv;

  return p->data;
}

void NOPH_MemoryFile_setup(FILE *out, void *ptr, size_t size, int allocate)
{
  NOPH_Memory_file_t *p;

  p = (NOPH_Memory_file_t*)out->priv;

  /* Fill in the private part of the FILE object */
  p->fp = 0;
  p->allocate = allocate;
  if (ptr == NULL)
    {
      p->data = malloc(4096);
      p->data_size = 0;
      p->allocated_size = 4096;
      p->allocate = 1;
      if (!p->data)
        {
          cibyl_file_free(out);
          /* FIXME: throw an OOM exception here */
          return;
        }
    }
  else
    {
      p->data = ptr;
      p->data_size = p->allocated_size = size;
    }
}

FILE *NOPH_MemoryFile_open(void *ptr, size_t size, int allocate)
{
  FILE *out;
  NOPH_Memory_file_t *p;

  /* Get a new FILE object */
  out = cibyl_file_alloc(&NOPH_Memory_fops);
  p = (NOPH_Memory_file_t*)out->priv;

  NOPH_MemoryFile_setup(out, ptr, size, allocate);

  return out;
}

FILE *NOPH_MemoryFile_openIndirect(const char *name, const char *in_mode)
{
  cibyl_fops_open_mode_t mode = cibyl_file_get_mode(in_mode);
  FILE *tmp;
  FILE *out;
  void *data = NULL;
  size_t size = 0;

  NOPH_panic_if(mode == APPEND || mode == READ_APPEND, "Invalid memory file mode '%s'", in_mode);

  /* Read in all of the file in some cases */
  if (mode == READ || mode == READ_WRITE)
    {
      size_t n = 0;
      size_t file_size = 0;
      const size_t bufsize = 4096;

      tmp = fopen(name, "r");
      if (!tmp)
        return NULL;

      /* Read in all of the file */
      do
        {
          size += bufsize;
          data = realloc(data, size);
          if (!data)
            {
              fclose(tmp);
              return NULL;
            }
          n = fread(data, 1, bufsize, tmp);
          file_size += n;
        } while(n == bufsize);
      fclose(tmp);
      /* Make the size match the file size */
      if (size != file_size)
        data = realloc(data, file_size);
      size = file_size;
    }

  /* Open the memory file */
  out = NOPH_MemoryFile_open(data, size, 1);

  if (mode == WRITE || mode == READ_WRITE || mode == READ_TRUNCATE)
    {
      NOPH_Memory_file_t *p = (NOPH_Memory_file_t*)out->priv;

      p->writeback_path = strdup(name);
      p->mode = strdup(in_mode);
    }

  NOPH_panic_if(!out, "MemoryFile_open failed!"); /* Should never happen, but OK */
  return out;
}
