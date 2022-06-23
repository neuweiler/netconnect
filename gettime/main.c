struct Library *BattClockBase;

/// extract_arg
STRPTR extract_arg(STRPTR string, STRPTR buffer, LONG len, char sep)
{
   STRPTR ptr1, ptr2;

   strncpy(buffer, string, len);

   ptr1 = strchr(buffer, (sep ? sep : ' '));
   ptr2 = strchr(buffer, 9);

   if(ptr2 && ((ptr2 < ptr1) || !ptr1))
      ptr1 = ptr2;
   if(ptr1)
      *ptr1 = NULL;

   string += strlen(buffer);

   while(*string == ' ' || *string == 9 || (sep ? *string == sep : NULL))
      string++;

   return((*string ? string : NULL));
}

///

int main(int argc, char *argv[])
{
   BPTR fh;
   char buf[256];
//   const char *months="JanFebMarAprMayJunJulAugSepOctNovDec", *days="SunMonTueWedThuFriSat";
   STRPTR ptr;
   char year[10], month[10], day[10], time[20];
   ULONG sys_secs;
   struct DateTime dat_time;

   sprintf(buf, "TCP:%ls/daytime", (argc == 2 ? argv[1] : "localhost"));
   Printf("opening %ls\n", buf);
   if(fh = Open(buf, MODE_OLDFILE))
   {
      FGets(fh, buf, 255);
      Printf("daytime: '%ls'\n", buf);

//    format: 'Sat Sep 26 10:56:55 1998\r\n'

      ptr = buf;
      if(ptr = extract_arg(ptr, month, sizeof(month), NULL))   // skip dayname
      {
Printf("1: %ls\n", month);
         if(ptr = extract_arg(ptr, month, sizeof(month), NULL))
         {
Printf("2: %ls\n", month);
            if(ptr = extract_arg(ptr, day, sizeof(day), NULL))
            {
Printf("3: %ls\n", day);
               if(ptr = extract_arg(ptr, time, sizeof(time), NULL))
               {
Printf("4: %ls\n", time);
Printf("5: %ls\n", ptr);
                  ptr += 2;
                  ptr[2] = NULL;
                  strncpy(year, ptr, sizeof(year));

Printf("time: %ls, date: %ls-%ls-%ls\n", time, day, month, year);

                  sprintf(buf, "%ls-%ls-%ls", day, month, year);
                  dat_time.dat_Format  = FORMAT_DOS;
                  dat_time.dat_StrDate = buf;
                  dat_time.dat_StrTime = time;
                  if(StrToDate(&dat_time))
                  {
                     struct timerequest   *TimeReq       = NULL;
                     struct MsgPort       *TimePort      = NULL;
                     struct   Library  *TimerBase        = NULL;
                     struct timeval tv;

                     Printf("ds_days:%ld\n", dat_time.dat_Stamp.ds_Days);
                     Printf("ds_minutes:%ld\n", dat_time.dat_Stamp.ds_Minute);
                     sys_secs = dat_time.dat_Stamp.ds_Days*86400 + dat_time.dat_Stamp.ds_Minute*60 + atol(&time[6]);
                     Printf("sys_secs:%ld\n", secs);
                     if(BattClockBase = OpenResource("battclock.resource"))
                     {
                        Printf("battclock: %ld\n", ReadBattClock());
                     }
                     if(TimePort = (struct MsgPort *)CreateMsgPort())
                     {
                        if(TimeReq = (struct timerequest *)CreateIORequest(TimePort, sizeof(struct timerequest)))
                        {
                           if(!(OpenDevice("timer.device", UNIT_VBLANK, (struct IORequest *)TimeReq, 0)))
                           {
                              TimerBase = &TimeReq->tr_node.io_Device->dd_Library;

                              GetSysTime(&tv);
                              Printf("tv: %ld\n", tv.tv_secs);

                              CloseDevice((struct IORequest *)TimeReq);
                           }
                           DeleteIORequest(TimeReq);
                        }
                        DeletePort(TimePort);
                     }
                  }
               }
            }
         }
      }
      Close(fh);
   }
   else Printf("couldn't open %ls\n", buf);
}

