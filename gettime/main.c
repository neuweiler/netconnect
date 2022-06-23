int main(int argc, char *argv[])
{
   BPTR fh;
   char buffer[256];

   sprintf(buffer, "TCP:%ls/daytime", (argc == 2 ? argv[1] : "localhost"));
   Printf("opening %ls\n", buffer);
   if(fh = Open(buffer, MODE_OLDFILE))
   {
      FGets(fh, buffer, 255);
      Printf("daytime: '%ls'\n", buffer);
      Close(fh);
   }
   else Printf("couldn't open %ls\n", buffer);
}

