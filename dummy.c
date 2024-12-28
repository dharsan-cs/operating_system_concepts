if (strcmp(string, eof) == 0) {
      write(processor_writer_pipe[write_end], string, strlen(string) + 1);
      return;
    }
    n = strlen(string);
    for (int i = 0; i < n / 2; i++) {
      temp = string[i];
      string[i] = string[n - i - 1];
      string[n - i - 1] = temp;
    }
    write(processor_writer_pipe[write_end], string, strlen(string) + 1);
    }