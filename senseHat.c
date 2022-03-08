#include "senseHat.h"

int estUnEventDevice (const struct dirent* dir)
 {
  return (int) ( strncmp ( "event", dir->d_name, strlen ("event")-1) == 0 );
 }

int estUnFrameBufferDevice (const struct dirent* dir )
 {
  return (int) (strncmp ("fb", dir->d_name, strlen("fb")-1) == 0 );
 }


SenseHat* SenseHat_creer ()
 {
  SenseHat* senseHat = ( SenseHat* ) malloc ( sizeof ( SenseHat ) );

  if ( senseHat )
   {
    struct dirent** listeDeNoms;

    //////////////////////////////////////////////// On recherche le joystick

    int nbEventDevice = scandir ( 
                                 "/dev/input"     ,    // Répertoire examiné
                                 &listeDeNoms     ,    // Liste des noms des event device trouvés
                                 estUnEventDevice ,    // Fonction qui filtre les entrées du répertoire (pour ne garder que les event devices)
                                 versionsort           // Fonction de trie des entrées (déjà fournie)
				);

    senseHat->evpoll.fd = -1;

    if ( nbEventDevice > 0 )
     {
      int numDevice;

      for ( numDevice = 0 ; numDevice < nbEventDevice ; numDevice++ )
       {		
        char nomDeFichier [64];

        snprintf (
                   nomDeFichier                   , 
                   sizeof (nomDeFichier)          ,  // On limite la taille du texte copié dans nomDeFichier à 64 (la taille du tableau) 
                   "/dev/input/%s"                ,  // On ajouter /dev/input devant le nom du device pour obtenir le nom du fichier
                   listeDeNoms[numDevice]->d_name    // le nom complet de l'event device
                 );
	
        int descripteurDeFichier = open (nomDeFichier, O_RDONLY);

        if ( descripteurDeFichier < 0)
           continue;

        char nom [256];

        ioctl ( 
	        descripteurDeFichier       , // Le descripteur de fichier désignant l'event device testé
                EVIOCGNAME ( sizeof(nom) ) , // Une fonction permettant de récupérer le nom de l'event device
                nom                          // Le pointeur vers la zone de stockage du nom
              );
	
        if ( strcmp ( "Raspberry Pi Sense HAT Joystick" ,  nom ) == 0 )
              senseHat->evpoll.fd = descripteurDeFichier;
         else close (descripteurDeFichier);
       }
     }

    for ( int numDevice = 0 ; numDevice < nbEventDevice ; numDevice++ )
     free ( listeDeNoms [numDevice] );
   
    senseHat->evpoll.events = POLLIN;
   

    ///////////////////////////////////////////////// Recherche de la dalle

        nbEventDevice = scandir (
                                 "/dev/"                ,    // Répertoire examiné
                                 &listeDeNoms           ,    // Liste des noms des event device trouvés
                                 estUnFrameBufferDevice ,    // Fonction qui filtre les entrées du répertoire (pour ne garder que les framebuffer devices)
	                         versionsort                 // Fonction de trie des entrées (déjà fournie)
				);

    senseHat->descripteurDalle = -1;

    if ( nbEventDevice > 0 )
     {
      struct fb_fix_screeninfo fix_info;

      for ( int numDevice = 0 ; numDevice < nbEventDevice ; numDevice++ )
       {
        char nomDeFichier [64];

        snprintf (
                  nomDeFichier                   , 
                  sizeof (nomDeFichier)          ,  // On limite la taille du texte copié dans nomDeFichier à 64 (la taille du tableau) 
                  "/dev/%s"                      ,  // On ajouter /dev/ devant le nom du device pour obtenir le nom du fichier
                  listeDeNoms[numDevice]->d_name    // le nom complet de l'event device
		 );

        int descripteurDeFichier = open (nomDeFichier, O_RDWR);

        if ( descripteurDeFichier < 0)
         continue;

        ioctl (
	        descripteurDeFichier , // Le descripteur de fichier désignant l'event device testé
                FBIOGET_FSCREENINFO  , // On demande les informations concernant la dalle RGB
                &fix_info              // Le pointeur vers la zone de stockage des informations demandées
              );

        if ( strcmp ( "RPi-Sense FB" , fix_info.id ) == 0 )
	      {
               senseHat->descripteurDalle = descripteurDeFichier;
               break;
	      }
         else close (descripteurDeFichier);
       }
     }
	 
    for ( int numDevice = 0 ; numDevice < nbEventDevice ; numDevice++ )
     free ( listeDeNoms [numDevice] );
    
    senseHat->adresseDuMappage = (uint16_t*)   mmap (
                                                      0                           ,  // On laisse le systeme determiner l'adresse 
                                                      128                         ,  // longueur de la plage d'adresse a mapper
                                                      PROT_READ | PROT_WRITE      ,  // On peut lire et ecrire dans la zone de memoire 
                                                      MAP_SHARED                  ,  // Partage de la projection avec d'autres processus ( ici l'API qui gere la dalle )
                                                      senseHat->descripteurDalle  ,  // descripteur de fichier
                                                      0                              // décalage dans le fichier 
                                                    );
 
 
   }

  if ( senseHat->evpoll.fd == -1 || senseHat->descripteurDalle == -1 || senseHat->adresseDuMappage == NULL )
   {
    printf ("Echec de configuration du sense-hat\n");

    SenseHat_detruire ( senseHat );

    senseHat = NULL;
   }

  return senseHat;
 }


void SenseHat_detruire ( SenseHat* s )
 {
  if ( s ) 
   {
    if ( s->evpoll.fd != -1 ) close ( s->evpoll.fd );

    if ( s->adresseDuMappage != NULL ) munmap ( s->adresseDuMappage , 128 );

    if ( s->descripteurDalle ) close ( s->descripteurDalle );

    free ( s );
   }
 }

int SenseHat_recupererEtatJoystick ( SenseHat* s )
 {
  if (s)
   {
    struct input_event ev[64];
    int i, rd;

    rd = read ( s->evpoll.fd , ev , sizeof(struct input_event) * 64 );
  
    if (rd < (int) sizeof(struct input_event)) 
     {
      return 0;
     }

    for (i = 0; i < rd / sizeof(struct input_event); i++) 
     {
      if (ev->type != EV_KEY)
       continue;

      if (ev->value != 1)
       continue;

      switch (ev->code) 
       {
        case KEY_ENTER :  return 5;
        case KEY_UP    :  return 1;
        case KEY_RIGHT :  return 2;
        case KEY_DOWN  :  return 3;
        case KEY_LEFT  :  return 4;
       }
     }
   }
  return 0;
 }

void SenseHat_modifierEtatLED ( SenseHat* s ,
                                int       _x ,
                                int       _y ,
                                int       _r , 
                                int       _v ,
                                int       _b )
 {
  if ( s )
   {
    unsigned int x,y,r,v,b;

    uint16_t val;

    if ( _x >   8 ) _x =   8; 
    if ( _x <   1 ) _x =   1;

    if ( _y >   8 ) _y =   8; 
    if ( _y <   1 ) _y =   1;

    if ( _r > 100 ) _r = 100; 
    if ( _r <   0 ) _r =   0;

    if ( _v > 100 ) _v = 100; 
    if ( _v <   0 ) _v =   0;

    if ( _b > 100 ) _b = 100; 
    if ( _b <   0 ) _b =   0;

    x = _x - 1;  
    y = _y - 1; 
  
    r = (unsigned int) ( _r * 31.0 / 100.0);
    v = (unsigned int) ( _v * 63.0 / 100.0);
    b = (unsigned int) ( _b * 31.0 / 100.0);
  
    val = (r << 11) + (v << 5) + b;
   
    *( s->adresseDuMappage + y * 8 + x ) = val;
   }
 }

void SenseHat_initialiserDalle ( SenseHat* s )
 {
  if ( s )
   {
    memset ( s->adresseDuMappage, 0, 128); 
   }
 }

