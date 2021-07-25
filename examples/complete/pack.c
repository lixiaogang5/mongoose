// Copyright (c) Cesanta Software Limited
// All rights reserved.

// This program is used to pack arbitrary data into a C binary. It takes
// a list of files as an input, and produces a .c data file that contains
// contents of all these files as a collection of byte arrays.
//
// Usage:
//   1. Compile this file:
//      cc -o pack pack.c
//
//   2. Convert list of files into single .c:
//      ./pack file1.data file2.data > fs.c
//
//   3. In your application code, you can access files using this function:
//      const char *unpack(const char *file_name, size_t *size);
//
//   4. Build your app with fs.c:
//      cc -o my_app my_app.c fs.c

#include <stdio.h>
#include <stdlib.h>

static const char *code =
    "const char *unpack(const char *name, size_t *size) {\n"
    "  const struct packed_file *p;\n"
    "  for (p = g_packed_files; p->name != NULL; p++) {\n"
    "    if (strcmp(p->name, name) != 0) continue;\n"
    "    if (size != NULL) *size = p->size - 1;\n"
    "    return (const char *) p->data;\n"
    "  }\n"
    "  return NULL;\n"
    "}\n";

int main(int argc, char *argv[]) {
  int i, j, ch;

  printf("%s", "#include <stddef.h>\n");
  printf("%s", "#include <string.h>\n");
  printf("%s", "\n");

  for (i = 1; i < argc; i++) {
    FILE *fp = fopen(argv[i], "rb");
    char ascii[12];
    if (fp == NULL) exit(EXIT_FAILURE);

    printf("static const unsigned char v%d[] = {\n", i);
    for (j = 0; (ch = fgetc(fp)) != EOF; j++) {
      if (j == (int) sizeof(ascii)) {
        printf(" // %.*s\n", j, ascii);
        j = 0;
      }
      ascii[j] = (ch >= ' ' && ch <= '~' && ch != '\\') ? ch : '.';
      printf(" %3u,", ch);
    }
    // Append zero byte at the end, to make text files appear in memory
    // as nul-terminated strings.
    // printf(" 0 // %.*s\n", (int) sizeof(ascii), ascii);
    printf(" 0 // %.*s\n};\n", j, ascii);
    fclose(fp);
  }

  printf("%s", "\nconst struct packed_file {\n");
  printf("%s", "  const char *name;\n");
  printf("%s", "  const unsigned char *data;\n");
  printf("%s", "  size_t size;\n");
  printf("%s", "} g_packed_files[] = {\n");

  for (i = 1; i < argc; i++) {
    printf("  {\"%s\", v%d, sizeof(v%d) },\n", argv[i], i, i);
  }
  printf("%s", "  {NULL, NULL, 0}\n");
  printf("%s", "};\n\n");
  printf("%s", code);

  return EXIT_SUCCESS;
}
