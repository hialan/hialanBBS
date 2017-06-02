/*-------------------------------------------------------*/
/*cut_ansistr()﹃糴, 琵肚﹃==┮璶―糴  */
/*buf把计肚芭奔び场﹃.                         */
/*str把计矪瞶﹃                                */
/*len把计                                  */
/*countê肚琌矪瞶筁=strip_ansi(buf,str,0)    */
/*:herb                                              */
/*-------------------------------------------------------*/

int
cut_ansistr(char *buf, char *str, int len) {
  register int ansi,count=0;

  for (ansi = 0; *str; str++) {
    if (*str == 27) {
      ansi = 1;
    }
    else if (ansi) {
      if (!strchr("[01234567;", *str)) {
         if (*str=='m' || *str=='*') {
            str--;
            for(;*str && strchr("[01234567;", *str) ;str--);
            for(;*str != 'm' && *str != '*';str++) {
              if (buf) *buf++ = *str;
            }
            if (buf) *buf++ = *str;
         }
         ansi = 0;
      }
    }
    else {
      if (count < len) {
        if (buf) *buf++ = *str;
        count++;
      }
    }
  }
  if (buf) *buf = '\0';
  return count;
}
