
#include "allegro5/allegro.h"
#include "allegro5/allegro_image.h"
#include <allegro5/allegro_primitives.h>
#include "allegro5/allegro_native_dialog.h"

short m_box(const char* message       = "No message given",
            const char* content_title = "Error",
            const char* title         = "Error"){

   switch(al_show_native_message_box(al_get_current_display(),
                                     title,
                                     content_title,
                                     message, NULL,
                                     ALLEGRO_MESSAGEBOX_YES_NO)){

      case 0: return 0; // User pressed the Cancel button
      case 1: return 1; //  "     "      "    OK     "
      case 2: return 2; // I don't understand what does this

   }
}

ALLEGRO_BITMAP *make_err_ima(){

   ALLEGRO_BITMAP *T_img = al_create_bitmap(32,32);
   if(!T_img)return 0;
   al_set_target_bitmap(T_img);
   al_clear_to_color(al_map_rgb(255, 0, 0));
   al_draw_line( 0,  0, 32, 32, al_map_rgb(255, 255, 255), 1.5);
   al_draw_line(32,  0,  0, 32, al_map_rgb(255, 255, 255), 1.5);
   al_set_target_bitmap(al_get_backbuffer(al_get_current_display()));
   return T_img;

}

ALLEGRO_BITMAP *smart_ima_loader(const char *image_name){

   ALLEGRO_BITMAP *T_img = NULL;
   T_img = al_load_bitmap(image_name);

   if(!T_img)
      switch(m_box("There is a proble when loading an image,"
             "would you like to continue anyway?", image_name)){

         case 0: return NULL; break;
         case 1: return make_err_ima(); break;

      }

   else
      return T_img;

}

int main(){

   ALLEGRO_DISPLAY *display = NULL;
   ALLEGRO_BITMAP  *image   = NULL;

   if(!al_init()){
      m_box("Problems when initilizing Allegro");
   }

   if(!al_init_image_addon()){
      m_box("Problems when initilizing the image addon");
   }

   if(!al_init_primitives_addon()){
      m_box("Problems when initilizing the primitives addon");
   }

   display = al_create_display(800,600);

   if(!display){
      m_box("Problems when creating the display");
   }
   //Since all this initializations are very necessary, there is no much we can do in case they fail

   image = smart_ima_loader("image.png");

   if(!image){
      return 0;
   }

   al_draw_bitmap(image, 200, 200, 0);

   al_flip_display();

   al_rest(2);

   al_destroy_bitmap(image);

   return 0;
}
