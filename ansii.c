#include <stdio.h>
#include <stdlib.h>
// #include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include "log.h"

/*  see `enum ansii_type` in header */
#include "ansii.h"

#define HASH_CHECK(a)    (key_hash == a)

struct ansii_t {
       char prefix_ch;
       struct {
              unsigned int single;
              unsigned int pair[2];
              unsigned int triplet[3];
       } value;
       char suffix_ch;
       enum {
              VALUE_SINGLE,
              VALUE_PAIR,
              VALUE_TRIPLET
       } value_type;
       bool valid;
       bool no_bracket;
};


unsigned long
hash(char str[])
{
       unsigned char* ustr = (unsigned char*)str;
       unsigned long hash = 5381;
       int c;

       while ((c = *ustr++))
              hash = ((hash << 5) + hash) + c; // hash * 33 + c 

       return hash;
}

struct ansii_t 
erase_to_ansii(char* key){
       unsigned long key_hash = hash(key);
       static bool hashes_initialized = false;

       static unsigned long  hash_erase_from_cursor_to_end, hash_erase_from_cursor_to_beginning,
                             hash_erase_entire_screen, hash_erase_saved_lines, erase_from_cursor_to_endline,
                             erase_from_start_line_to_cursor, erase_entire_line;
                            

       if(!hashes_initialized){
              log_info("initializing hashes\n");
              hash_erase_from_cursor_to_end = hash("from-cursor-to-end");
              hash_erase_from_cursor_to_beginning = hash("from-cursor-to-beginning");
              hash_erase_entire_screen = hash("entire-screen");
              hash_erase_saved_lines = hash("saved-lines");
              erase_from_cursor_to_endline = hash("from-cursor-to-endline");
              erase_from_start_line_to_cursor = hash("from-start-line-to-cursor");
              erase_entire_line = hash("entire-line");
              hashes_initialized = true;
       }
       if HASH_CHECK(hash_erase_from_cursor_to_end)       return (struct ansii_t){.value.single=0,.suffix_ch='J',.valid=true};
       if HASH_CHECK(hash_erase_from_cursor_to_beginning) return (struct ansii_t){.value.single=1,.suffix_ch='J',.valid=true};
       if HASH_CHECK(hash_erase_entire_screen)            return (struct ansii_t){.value.single=2,.suffix_ch='J',.valid=true};
       if HASH_CHECK(hash_erase_saved_lines)              return (struct ansii_t){.value.single=3,.suffix_ch='J',.valid=true};
       if HASH_CHECK(erase_from_cursor_to_endline)        return (struct ansii_t){.value.single=0,.suffix_ch='K',.valid=true};
       if HASH_CHECK(erase_from_start_line_to_cursor)     return (struct ansii_t){.value.single=1,.suffix_ch='K',.valid=true};
       if HASH_CHECK(erase_entire_line)                   return (struct ansii_t){.value.single=2,.suffix_ch='K',.valid=true};
       log_error("Invalid value: '%s'\n",key);
       return (struct ansii_t){.valid=false};
}


struct ansii_t
cursor_to_ansii(char* key){
       unsigned long key_hash = hash(key);
       static bool hashes_initialized = false;

       static unsigned long hash_visible, hash_invisible, hash_to_origin, hash_one_up, hash_save,
                            hash_restore, hash_request_position;
       unsigned int* pair_to_read = malloc(sizeof(unsigned int)*2);
       unsigned int value_to_read;

       if(!hashes_initialized){
              log_info("initializing hashes\n");
              hash_visible = hash("visible");
              hash_invisible = hash("invisible");
              hash_to_origin = hash("to-origin");
              hash_one_up = hash("one-up");
              hash_request_position = hash("request-position");
              hash_save = hash("save");
              hash_restore = hash("restore");
              hashes_initialized = true;
       }
       // log_verbose("hash: %lu of (%lu; %lu)",key_hash,hash_visible, hash_invisible);
       if HASH_CHECK(hash_invisible)        return (struct ansii_t){'?',.value.single=25,.suffix_ch='l',.valid=true}; // ?25h
       if HASH_CHECK(hash_visible)          return (struct ansii_t){'?',.value.single=25,.suffix_ch='h',.valid=true};  // ?25l
       if HASH_CHECK(hash_restore)          return (struct ansii_t){.suffix_ch='u',.valid=true};  // ?25l
       if HASH_CHECK(hash_to_origin)        return (struct ansii_t){.suffix_ch='H',.valid=true};
       if HASH_CHECK(hash_one_up)           return (struct ansii_t){.suffix_ch='M',.valid=true,.no_bracket=true};
       if HASH_CHECK(hash_save)             return (struct ansii_t){.suffix_ch='s',.valid=true};
       if HASH_CHECK(hash_restore)          return (struct ansii_t){.suffix_ch='u',.valid=true};
       if HASH_CHECK(hash_request_position) return (struct ansii_t){.value.single=6,.suffix_ch='n',.valid=true};
       // finished static analisys
       struct ansii_t result = {};
       if(sscanf(key,"move-to-%u-%u",&pair_to_read[0],&pair_to_read[1])){
              result.suffix_ch = 'H';
              result.valid = true;
              if(memmove(result.value.pair,pair_to_read,2)){
                     return result;
              }
              log_error("Memmove failed. pair not copied over");
       } else if(sscanf(key,"move-%u-up",&value_to_read)){    return (struct ansii_t){.value.single=value_to_read,.suffix_ch='A',.valid=true};
       } else if(sscanf(key,"move-%u-down",&value_to_read)){  return (struct ansii_t){.value.single=value_to_read,.suffix_ch='B',.valid=true};
       } else if(sscanf(key,"move-%u-right",&value_to_read)){ return (struct ansii_t){.value.single=value_to_read,.suffix_ch='C',.valid=true};
       } else if(sscanf(key,"move-%u-left",&value_to_read)){  return (struct ansii_t){.value.single=value_to_read,.suffix_ch='D',.valid=true};
       } else if(sscanf(key,"move-to-beginning-%u-lines-down",&value_to_read)){ return (struct ansii_t){.value.single=value_to_read,.suffix_ch='E',.valid=true};
       } else if(sscanf(key,"move-to-beginning-%u-lines-up",&value_to_read)){   return (struct ansii_t){.value.single=value_to_read,.suffix_ch='F',.valid=true};
       } else if(sscanf(key,"move-to-col-%u",&value_to_read)){                  return (struct ansii_t){.value.single=value_to_read,.suffix_ch='G',.valid=true};
       }
       log_error("Invalid value: '%s'\n",key);
       return (struct ansii_t){.valid=false};
}

struct ansii_t
altbuf_to_ansii(char* key){
       unsigned long key_hash = hash(key);
       static bool hashes_initialized = false;

       static unsigned long hash_enable, hash_disable;
       if(!hashes_initialized){
              log_info("initializing hashes\n");
              hash_enable = hash("enable");
              hash_disable = hash("disable");
              hashes_initialized = true;
       }
       if HASH_CHECK(hash_enable)  return (struct ansii_t){'?',.value.single = 1049,.suffix_ch = 'h',.valid=true};
       if HASH_CHECK(hash_disable) return (struct ansii_t){'?',.value.single = 1049,.suffix_ch = 'l',.valid=true};
       log_error("Invalid value: '%s'\n",key);
       return (struct ansii_t){.valid=false};
}

struct ansii_t
scattr_to_ansii(char* key){
       unsigned long key_hash = hash(key);
       static bool hashes_initialized = false;

       static unsigned long hash_screen_40_x_25_monochrome_text, hash_screen_40_x_25_color_text, hash_screen_80_x_25_monochrome_text, hash_screen_80_x_25_color_text, hash_screen_320_x_200_4_color_graphics, hash_screen_320_x_200_monochrome_graphics, hash_screen_640_x_200_monochrome_graphics, hash_screen_linewrap, hash_screen_320_x_200_color_graphics, hash_screen_640_x_350_2_color_graphics, hash_screen_640_x_350_16_color_graphics, hash_screen_640_x_480_2_color_graphics, hash_screen_640_x_480_16_color_graphics, hash_screen_320_x_200_256_color_graphics, hash_screen_restore, hash_screen_save;



       if(!hashes_initialized){
              log_info("initializing hashes\n");
              hash_screen_40_x_25_monochrome_text        = hash("40x25 monochrome (text)");
              hash_screen_40_x_25_color_text             = hash("40x25 color (text)");
              hash_screen_80_x_25_monochrome_text        = hash("80x25 monochrome (text)");
              hash_screen_80_x_25_color_text             = hash("80x25 color (text)");
              hash_screen_320_x_200_4_color_graphics     = hash("320x200 4-color (graphics)");
              hash_screen_320_x_200_monochrome_graphics  = hash("320x200 monochrome (graphics)");
              hash_screen_640_x_200_monochrome_graphics  = hash("640x200 monochrome (graphics)");
              hash_screen_linewrap                       = hash("linewrap");
              hash_screen_320_x_200_color_graphics       = hash("320x200 color (graphics)");
              hash_screen_640_x_350_2_color_graphics     = hash("640x350 monochrome (2-color graphics)");
              hash_screen_640_x_350_16_color_graphics    = hash("640x350 color (16-color graphics)");
              hash_screen_640_x_480_2_color_graphics     = hash("640x480 monochrome (2-color graphics)");
              hash_screen_640_x_480_16_color_graphics    = hash("640x480 color (16-color graphics)");
              hash_screen_320_x_200_256_color_graphics   = hash("320x200 color (graphics)");
              hash_screen_restore                        = hash("restore");
              hash_screen_save                           = hash("save");
              hashes_initialized = true;
       }

       // log_info("key_hash: %lu, should be %lu\n", key_hash, hash_screen_40_x_25_monochrome_text);

       if HASH_CHECK(hash_screen_40_x_25_monochrome_text)       return (struct ansii_t){'=',.value.single=0,.suffix_ch='h',.valid=true}; // 40 x 25 monochrome (text)
       if HASH_CHECK(hash_screen_40_x_25_monochrome_text)       return (struct ansii_t){'=',.value.single=1,.suffix_ch='h',.valid=true}; // 40 x 25 color (text)
       if HASH_CHECK(hash_screen_40_x_25_color_text)            return (struct ansii_t){'=',.value.single=2,.suffix_ch='h',.valid=true}; // 80 x 25 monochrome (text)
       if HASH_CHECK(hash_screen_80_x_25_monochrome_text)       return (struct ansii_t){'=',.value.single=3,.suffix_ch='h',.valid=true}; // 80 x 25 color (text)
       if HASH_CHECK(hash_screen_80_x_25_color_text)            return (struct ansii_t){'=',.value.single=4,.suffix_ch='h',.valid=true}; // 320 x 200 4-color (graphics)
       if HASH_CHECK(hash_screen_320_x_200_4_color_graphics)    return (struct ansii_t){'=',.value.single=5,.suffix_ch='h',.valid=true}; // 320 x 200 monochrome (graphics)
       if HASH_CHECK(hash_screen_320_x_200_monochrome_graphics) return (struct ansii_t){'=',.value.single=6,.suffix_ch='h',.valid=true}; // 640 x 200 monochrome (graphics)
       if HASH_CHECK(hash_screen_640_x_200_monochrome_graphics) return (struct ansii_t){'=',.value.single=7,.suffix_ch='h',.valid=true}; // linewrap
       if HASH_CHECK(hash_screen_linewrap)                      return (struct ansii_t){'=',.value.single=13,.suffix_ch='h',.valid=true}; //320 x 200 color (graphics)
       if HASH_CHECK(hash_screen_320_x_200_color_graphics)      return (struct ansii_t){'=',.value.single=14,.suffix_ch='h',.valid=true}; //640 x 200 color (16-color graphics)
       if HASH_CHECK(hash_screen_640_x_350_2_color_graphics)    return (struct ansii_t){'=',.value.single=15,.suffix_ch='h',.valid=true}; //640 x 350 monochrome (2-color graphics)
       if HASH_CHECK(hash_screen_640_x_350_16_color_graphics)   return (struct ansii_t){'=',.value.single=16,.suffix_ch='h',.valid=true}; //640 x 350 color (16-color graphics)
       if HASH_CHECK(hash_screen_640_x_480_2_color_graphics)    return (struct ansii_t){'=',.value.single=17,.suffix_ch='h',.valid=true}; //640 x 480 monochrome (2-color graphics)
       if HASH_CHECK(hash_screen_640_x_480_16_color_graphics)   return (struct ansii_t){'=',.value.single=18,.suffix_ch='h',.valid=true}; //640 x 480 color (16-color graphics)
       if HASH_CHECK(hash_screen_320_x_200_256_color_graphics)  return (struct ansii_t){'=',.value.single=19,.suffix_ch='h',.valid=true}; //320 x 200 color (256-color graphics)
       if HASH_CHECK(hash_screen_restore)                       return (struct ansii_t){'?',.value.single=47,.suffix_ch='l',.valid=true}; //restore
       if HASH_CHECK(hash_screen_save)                          return (struct ansii_t){'?',.value.single=47,.suffix_ch='h',.valid=true}; //restore
       // case : return (struct ansii_t){'?',47,'h'}; //save
       log_error("Invalid value: '%s'\n",key);
       return (struct ansii_t){.valid=false};
}



unsigned int
fntattr_to_ansii(char* font_attr){
       unsigned long key_hash = hash(font_attr);
       static bool hashes_initialized = false;
       static unsigned long ansii_hash_fontattr_bold, ansii_hash_fontattr_faint, ansii_hash_fontattr_italic, ansii_hash_fontattr_underline, ansii_hash_fontattr_blink, ansii_hash_fontattr_inverse, ansii_hash_fontattr_invisible, ansii_hash_fontattr_strikethrough;

       if(!hashes_initialized){
              log_info("initializing hashes\n");
              ansii_hash_fontattr_bold          = hash("bold");
              ansii_hash_fontattr_faint         = hash("faint");
              ansii_hash_fontattr_italic        = hash("italic");
              ansii_hash_fontattr_underline     = hash("underline");
              ansii_hash_fontattr_blink         = hash("blink");
              ansii_hash_fontattr_inverse       = hash("inverse");
              ansii_hash_fontattr_invisible     = hash("invisible");
              ansii_hash_fontattr_strikethrough = hash("strikethrough");
              hashes_initialized = true;
       }

       if HASH_CHECK(ansii_hash_fontattr_bold)         return 1;
       if HASH_CHECK(ansii_hash_fontattr_faint)        return 2;
       if HASH_CHECK(ansii_hash_fontattr_italic)       return 3;
       if HASH_CHECK(ansii_hash_fontattr_underline)    return 4;
       if HASH_CHECK(ansii_hash_fontattr_blink)        return 5;
       if HASH_CHECK(ansii_hash_fontattr_inverse)      return 6;
       if HASH_CHECK(ansii_hash_fontattr_invisible)    return 7;
       if HASH_CHECK(ansii_hash_fontattr_strikethrough)return 8;
       log_error("Invalid font attribute: `%s`\n", font_attr);
       return 0; // reset
}
unsigned int
color_to_ansii(char* color) {
       unsigned long color_hash = hash(color);
       static bool hashes_initialized = false;
       static unsigned long hash_color_black, hash_color_red, hash_color_green, hash_color_yellow, hash_color_blue, hash_color_magenta, hash_color_cyan, hash_color_white;
       if(!hashes_initialized){
              log_info("initializing hashes\n");
              hash_color_black = hash("black");
              hash_color_red   = hash("red");
              hash_color_green = hash("green");
              hash_color_yellow= hash("yellow");
              hash_color_blue  = hash("blue");
              hash_color_magenta = hash("magenta");
              hash_color_cyan  = hash("cyan");
              hash_color_white = hash("white");
              hashes_initialized = true;
       }
       if(color_hash == hash_color_black)   return 0;
       if(color_hash == hash_color_red)     return 1;
       if(color_hash == hash_color_green)   return 2;
       if(color_hash == hash_color_yellow)  return 3;
       if(color_hash == hash_color_blue)    return 4;
       if(color_hash == hash_color_magenta) return 5;
       if(color_hash == hash_color_cyan)    return 6;
       if(color_hash == hash_color_white)   return 7;
       return 9; // default
}

struct ansii_t
parse_ansii_type(enum ansii_types ansii_type, char* value){
       struct ansii_t ansii_r;
       switch(ansii_type){
              case BG:
                     ansii_r.value.single = 40 + color_to_ansii(value);
                     ansii_r.suffix_ch = 'm';
                     ansii_r.valid = true;
                     break;
              case FG:
                     ansii_r.value.single = 30 + color_to_ansii(value);
                     ansii_r.suffix_ch = 'm';
                     ansii_r.valid = true;
                     break;
              case FONT_SET:
                     ansii_r.value.single = fntattr_to_ansii(value);
                     ansii_r.valid = true;
                     break;
              case FONT_RESET:
                     ansii_r.value.single = 20 + fntattr_to_ansii(value);
                     ansii_r.valid = true;
                     break;
                     // more complicated cases that, aren't just based on value.
              case SCREEN: return scattr_to_ansii(value);
              case CURSOR: return cursor_to_ansii(value);
              case ALTBUF: return altbuf_to_ansii(value);
              case ERASE: return erase_to_ansii(value);
              case RESET:
                     ansii_r.value.single = 0;
                     ansii_r.valid = true;
                     break;

       }
       return ansii_r;
}
int 
ansii_transform(FILE* from, FILE* to){
       // this assumes file `from` is readable, and `to`
       // is writable, and not equal. Make sure of this before calling this function.
#define BUFSIZE 1024
       char buf[BUFSIZE];
       char* save_ptr_vkey;

       enum ansii_types ansii_type;
       unsigned int read;
       struct ansii_t ansii_result;
       char* token_complete;
       char* end;
       char string_to_output[8];
       static bool initialized_hashes = false;
       unsigned int hash_bg, hash_fg, hash_font_set, hash_font_reset, hash_screen, hash_cursor, 
                    hash_altbuf, hash_erase, hash_reset;
       unsigned int key_hash;

       if(!initialized_hashes){
              hash_bg         = hash("bg");
              hash_fg         = hash("fg");
              hash_font_set   = hash("font-set");
              hash_font_reset = hash("font-reset");
              hash_screen     = hash("screen");
              hash_cursor     = hash("cursor");
              hash_altbuf     = hash("altbuf");
              hash_erase      = hash("erase");
              hash_reset      = hash("reset");
              
              initialized_hashes = true;
       }

       while((read = fread(buf,sizeof(char),BUFSIZE,from))){
              // char* token_complete = strtok_r(buf,"{}",&save_ptr_buf);
              log_verbose("buffer read %u characters\n",read);
              token_complete = buf;
              while((token_complete = strstr(token_complete, "{{")) != NULL){
                     token_complete += 2;
                     end = strstr(token_complete,"}}");
                     if(!end){
                            break;
                     }
                     *end = 0;
                     char* key = strtok_r(token_complete,":",&save_ptr_vkey);
                     key_hash = hash(key);

                     if HASH_CHECK(hash_bg){                ansii_type = BG;
                     } else if HASH_CHECK(hash_fg){         ansii_type = FG;
                     } else if HASH_CHECK(hash_font_set){   ansii_type = FONT_SET;
                     } else if HASH_CHECK(hash_font_reset){ ansii_type = FONT_RESET; 
                     } else if HASH_CHECK(hash_screen){     ansii_type = SCREEN; 
                     } else if HASH_CHECK(hash_cursor){     ansii_type = CURSOR; 
                     } else if HASH_CHECK(hash_altbuf){     ansii_type = ALTBUF;
                     } else if HASH_CHECK(hash_erase){      ansii_type = ERASE;
                     } else if HASH_CHECK(hash_reset){      ansii_type = RESET; }
                     char* value = strtok_r(NULL,":",&save_ptr_vkey);
                     ansii_result = parse_ansii_type(ansii_type,value);

                     if(ansii_result.valid){
                            switch(ansii_result.value_type){
                                   case VALUE_SINGLE:
                                          if(ansii_result.prefix_ch){
                                                 fprintf(to,"\e[%c%d%c\n", ansii_result.prefix_ch, ansii_result.value.single, ansii_result.suffix_ch);
                                          } else {
                                                 fprintf(to,"\e[%d%c\n", ansii_result.value.single, ansii_result.suffix_ch);
                                          }
                                          break;
                                          // TODO: VALUE_PAIR
                                          // TODO: VALUE_TRIPLET
                                   default:
                                          break;
                            }
                     }
                     // go until { and take the token
                     token_complete = end + 2;
              }
       }
       return 0;

#undef BUFSIZE
}


