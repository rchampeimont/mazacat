/*
Copyright (c) 2006, 2012  RIVER CHAMPEIMONT
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

    * Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above
copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the
distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <stdlib.h>
#include <string.h>
#include <math.h>


// Librairies requises : SDL et SDL_image
#include "SDL.h"
#include "SDL_image.h"
/*
This program uses the SDL (Simple DirectMedia Layer) library, licensed under the GNU Lesser General Public License (LGPL).
SDL Website : www.libsdl.org
*/


#define MAZACAT_VERSION "20060418"


/*
FRANCAIS:
Information pour les programmeurs :
MAZACAT est organise comme ceci :

Le dossier "niveaux" contient des groupes de niveaux sous forme de dossiers
nommes par un nombre et l'extenstion GRP :
niveaux/
 - 00000001.grp/
 - 00000002.grp/
 - 00000003.grp/ ...

Chaque dossier correspond a un groupe de niveux qui utilisent les MEMES IMAGES
Chaque dossier contient :
- images.png (le fichier qui contient les images de types de cases de labyrinthe pour ce groupe de niveaux)
- 00000001.niv
- 00000002.niv
- 00000003.niv ....  (chaque niveau est un nombre suivi de l'extension .niv)

Exemple : 
niveaux/00000001.grp/00000001.niv
niveaux/00000001.grp/00000002.niv  utilisent les memes images contenues dans 
niveaux/00000001.grp/images.png

mais
niveaux/00000001.grp/00000001.niv
niveaux/00000007.grp/00000001.niv utilisent respectivement les images de
niveaux/00000001.grp/images.png
niveaux/00000007.grp/images.png


Fromat des fichiers de niveau : ASCII
Ce qui compte sont les nombres hexadecimaux correspondant aux caracteres
c'est pourquoi les fichiers de niveaux (1, 2, 3...) sont illisibles
Pour permettre un nombre d'images de cases de labyrinthe possibles
tres eleves nous utilisons le systeme suivant :
Nous chargeons l'image contenue dans images.png aux coordonnees (0, 20*Nombre)
20 est la taille d'une case, Nombre est le nombre correspondant au caractere
Pour modifier les niveaux utilisez un editeur hexadeciaml :
Un fichier images.png est fait comme ceci :

 ^ /-------\
20 |codeHex|  <-- 1ere image
px |  00   |
 v \-------/
 ^ /-------\
20 |codeHex|
px |  01   |  <-- 2e image
 v \-------/


Exemple :
00 01 01 01
donne : 1ere-image 2e-image 2e-image 2e-image 

*/

// Image toutes les n millisecondes
// Physics iterations every milisecond
#define IMAGE_TOUTES_LES_N_MS 80    // 80 ms
// 12.5 FPS


/*
ImageParSeconde   IMAGE_TOUTES_LES_N_MS
FPS               frame every N ms
60                     16
50                     20
30                     33
20                     50
12.5                   80
10                    100
 4                    250
 1                   1000
*/


// Type d'affichage (display type)

#define AFFICHAGE SDL_FULLSCREEN     // plein ecran (fullscreen)
//#define AFFICHAGE SDL_SWSURFACE      // fenetre (window)

// resolution
#define XRES 800
#define YRES 600
// Premiere pronfondeur de couleur a essayer
#define PREMIERE_PROFONDEUR_COULEUR 32
// Seconde pronfondeur de couleur a essayer
#define SECONDE_PROFONDEUR_COULEUR 8
// varibale profonduer de couleur : egale a PREMIERE_PROFONDEUR_COULEUR ou
// SECONDE_PROFONDEUR_COULEUR
unsigned int ProfondeurCouleur;
// nombre d'entrees dans le tableau touchesEnfoncees
#define nombreTouchesMax 200


SDL_Surface *screen;

// Surfaces :
// this one contains the maze images
// celle-ci contient les images de cases du labyrinthe
SDL_Surface *imagesCasesSurface;
// this one contains the full maze image (the background of the game)
// cette surface contient l'image du labyrinthe entier (arriere plan du jeu)
// remarque : maze signifie labyrinthe, nous avons choisi un mot anglais
// en hommage a MAZE-A (qui est pourtant un programme francais)
SDL_Surface *mazeSurface;

// pitch (voir doc SDL : sous UNIX : man SDL_Surface)
#define PITCH (screen->pitch / 4)



// tick (voir doc SDL : sous UNIX : man SDL_GetTicks)
unsigned int tick;

// pour le controle du rafraichissement : numero du dernier tick
unsigned int dernierTick;

// varibales globales : (global variables)
// animal gagnant  (winner animal)
signed int gagnant;


/* pourquoi 13 caracteres ? parce que ce sont des noms de fichiers
8 pour le nom, 1 pour le point, 3 pour l'extension
et 1 pour le caractere de fin = 13 caracteres
12345678.123F   -> 8 + 1 + 3 + 1 = 13
*/
unsigned long grp ;
char niveauGRP[13];
unsigned long niv ;
char niveauNIV[13];


char niveauFichier[40];
// niveaux + / + niveauGRP + / + niveauNIV + carac_fin
// 7      + 1 +   12      + 1 +   12      +  1        = 34
//(on prend 40 pour avoir a nombre rond et puis faut pas etre radin )
char imagesMazeFichier[40];


unsigned short controleAnimal1 ;
// controles du chat
// 0 = ZQSD
// 1 = WASD
// 2 = souris


int nombreDe8ChiffresVersChaineAvecZeros(unsigned long n, char *s) {
  
  unsigned long k;
  unsigned short a;
  strcpy(s, "");

  k = 10000000 ;
  
  while (k>=1)
    {
      a = n / k ;
      
      switch (a) {
      case 0 :
	strcat(s, "0");
	break;
      case 1 :
	strcat(s, "1");
	break;
      case 2 :
	strcat(s, "2");
	break;
      case 3 :
	strcat(s, "3");
	break;
      case 4 :
	strcat(s, "4");
	break;
      case 5 :
	strcat(s, "5");
	break;
      case 6 :
	strcat(s, "6");
	break;
      case 7 :
	strcat(s, "7");
	break;
      case 8 :
	strcat(s, "8");
	break;
      case 9 :
	strcat(s, "9");
	break;
      default :
	return 1 ;
      }
      
      n = n - a*k ;
      k = k /10 ;
    }
  
  
  return 0 ;
}


int nivgrp2nomsFichiers()
{
  strcpy(niveauGRP, "");
  strcpy(niveauNIV, "");
  strcpy(imagesMazeFichier, "");
  strcpy(niveauFichier, "");
  
  nombreDe8ChiffresVersChaineAvecZeros(grp, niveauGRP);
  strcat(niveauGRP, ".grp");

  nombreDe8ChiffresVersChaineAvecZeros(niv, niveauNIV);
  strcat(niveauNIV, ".niv");
  
  strcat(imagesMazeFichier, "niveaux/");
  strcat(imagesMazeFichier, niveauGRP);
  strcat(imagesMazeFichier, "/images.png");

  strcat(niveauFichier, "niveaux/");
  strcat(niveauFichier, niveauGRP);
  strcat(niveauFichier, "/");
  strcat(niveauFichier, niveauNIV);
  
  return 0 ;
}






int licence()
{
  printf("Copyright (c) 2006, RIVER CHAMPEIMONT\nAll rights reserved.\n\nRedistribution and use in source and binary forms, with or without\nmodification, are permitted provided that the following conditions are\nmet:\n\n    * Redistributions of source code must retain the above copyright\nnotice, this list of conditions and the following disclaimer.\n    * Redistributions in binary form must reproduce the above\ncopyright notice, this list of conditions and the following disclaimer\nin the documentation and/or other materials provided with the\ndistribution.\n\nTHIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS\n\"AS IS\" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT\nLIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR\nA PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT\nOWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,\nSPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT\nLIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\nDATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\nTHEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE\nOF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n\n");
  return 0 ;
}




// dessinerImageAvecVerrouillerEtRafraichir
int dessinerImageVR(SDL_Surface *surface, char *fichierImage, int positionX, int PositionY )
{
  
  
  // Verrouiller la surface si besoin (Lock surface if needed)
  if (SDL_MUSTLOCK(surface))
    if (SDL_LockSurface(surface) < 0) 
      return 0;
  
  
  
  SDL_Surface *image = IMG_Load ( fichierImage );
  
  
  
  if (!image) {
    fprintf(stderr, "Erreur lors du chargement de l'image : %s\n", fichierImage);
    fprintf(stderr, "SDL : %s\n", SDL_GetError());
    exit(1) ;
  }
  // Dessine l'image sur la surface
  SDL_Rect rcDest = { positionX, PositionY, 0, 0 };
  SDL_BlitSurface ( image, NULL, surface, &rcDest );
  SDL_FreeSurface ( image );
  
  
  // Deverrouiller si besoin (Unlock if needed)
  if (SDL_MUSTLOCK(surface)) 
    SDL_UnlockSurface(surface);
  
  
  // Rarfrachir la surface en entier
  
  SDL_UpdateRect(surface, 0, 0, 0, 0);  
  
  
  return 0;
  
  
}





// dessine un animal vers une surface (draws an animal to surface)
int dessinerAnimal(SDL_Surface *sourceSurface, SDL_Surface *destinationSurface, int positionX, int PositionY )
{

  // Dessine l'image sur la surface (Draws the image on the surface) :
  SDL_Rect rcDest = { positionX, PositionY, 20, 20 };
  SDL_BlitSurface ( sourceSurface, NULL, destinationSurface, &rcDest );
  

  return 0;
}




// la fonction fin() est appelle quand un signal TERM est recu (the fin() funcion is called when a TERM signal is received)
int fin()
{
  printf("Vous voulez quitter, alors au revoir !\n");
  exit(0) ;
}




int dessinerCase(int x, int y, int numeroDuTypeDeCase, SDL_Surface *destinationSurface)
{
  // Verrouiller la surface si besoin (Lock surface if needed)
  if (SDL_MUSTLOCK(imagesCasesSurface))
    if (SDL_LockSurface(imagesCasesSurface) < 0) 
      return 1;

  int i, j;
  for (i = 0; i < 20; i++)
  {

    int destinationSurfaceOfs = (x / (32/ProfondeurCouleur) + (y + i) * PITCH);
    int caseOfs = (i + numeroDuTypeDeCase * 20) * imagesCasesSurface->pitch / 4 ;

    for (j = 0; j < 20 / (32/ProfondeurCouleur); j++)
    {

      ((unsigned int*)destinationSurface->pixels)[destinationSurfaceOfs] = 
        ((unsigned int*)imagesCasesSurface->pixels)[caseOfs];
      destinationSurfaceOfs++;
      caseOfs++;
    }
  }

  // Deverrouiller si besoin (Unlock if needed)
  if (SDL_MUSTLOCK(imagesCasesSurface)) 
    SDL_UnlockSurface(imagesCasesSurface);

  return 0;
}

int dessinerMenu(SDL_Surface *fondSurface, SDL_Surface *selecteurSurface, SDL_Surface *devantSurface, signed short choix, SDL_Surface *chcZqsdSurface, SDL_Surface *chcWasdSurface, SDL_Surface *chcSouriSurface) {
  
  // le fond
  SDL_BlitSurface(fondSurface, NULL, screen, NULL);
  // l'indicateur de choix (selecteur)
  SDL_Rect rcDest = { 0, 222 + 40*choix, 0, 0 };
  SDL_BlitSurface ( selecteurSurface, NULL, screen, &rcDest );
  // le texte
  SDL_BlitSurface(devantSurface, NULL, screen, NULL);
  // le texte pour le choix du controle du chien
  switch (controleAnimal1) {
  case 0 :
    SDL_BlitSurface(chcZqsdSurface, NULL, screen, NULL);
    break;
  case 1 :
    SDL_BlitSurface(chcWasdSurface, NULL, screen, NULL);
    break;
  case 2 :
    SDL_BlitSurface(chcSouriSurface, NULL, screen, NULL);
    break;
  default :
    exit(1);
    break;
  }
  // Rafraichir
  SDL_UpdateRect(screen, 0, 0, 0, 0);
  

  return 0 ;
}



int dessinerChoixGroupe(SDL_Surface *fondSurface, SDL_Surface *chiffresSurface, unsigned long grp, SDL_Surface *selecteurSurface, unsigned short sel) {
  
  // le fond
  SDL_BlitSurface(fondSurface, NULL, screen, NULL);

  // les chiffres
  unsigned int chiffreX;
  unsigned long diviseur;
  unsigned int a;
  unsigned long n;
  n = grp ;

  diviseur = 10000000 ;
  chiffreX = 10 ;
  
  while (diviseur>=1)
    {
      a = n / diviseur ;
      if (a>9) {
	return 1;
      } else {
	
	SDL_Rect rcSrc = {0, 60*a, 60, 60};
	SDL_Rect rcDest = {chiffreX, 370, 60, 60};
	SDL_BlitSurface ( chiffresSurface, &rcSrc, screen, &rcDest );
  
      }
      
      
      n = n - a*diviseur ;
      diviseur = diviseur / 10 ;
      chiffreX += 60;
    }

  // la fleche
  SDL_Rect rcSrc = {0, 0, 60, 60};
  SDL_Rect rcDest = {10 + 60*sel, 370+60, 60, 60};
  SDL_BlitSurface ( selecteurSurface, &rcSrc, screen, &rcDest );
  


  // Rafraichir
  SDL_UpdateRect(screen, 0, 0, 0, 0);
  
  return 0 ;
}


int dessinerVersion(SDL_Surface *chiffres30Surface) {
  
  // les chiffres
  unsigned int chiffreX;
  unsigned long diviseur;
  unsigned int a;
  unsigned long n;

  n = strtol(MAZACAT_VERSION,NULL,10) ;

  diviseur = 10000000 ;
  chiffreX = 560 ;
  
  while (diviseur>=1)
    {
      a = n / diviseur ;
      if (a>9) {
	return 1;
      } else {
	
	SDL_Rect rcSrc = {0, 30*a, 30, 30};
	SDL_Rect rcDest = {chiffreX, 570, 30, 30};
	SDL_BlitSurface ( chiffres30Surface, &rcSrc, screen, &rcDest );
  
      }
      
      
      n = n - a*diviseur ;
      diviseur = diviseur / 10 ;
      chiffreX += 30;
    }

  // Rafraichir
  SDL_UpdateRect(screen, 0, 0, 0, 0);
  
  return 0 ;
}


unsigned long changerUnChiffreDUnNombreDe8Chiffres(unsigned long n, unsigned short chiffre, bool plusOuMoins) {

  unsigned short positionChiffreActuel = 0 ;
  unsigned long nombreSortie = 0 ;
  unsigned long diviseur = 10000000  ;
  unsigned short a;
  unsigned short b;

  
  while (diviseur >= 1)
    {
      a = n / diviseur ;
      b = a;
      if (a > 9) {
	return 99999999;
      } else {
	if (positionChiffreActuel == chiffre) {
	  // cas : le chiffre qu'on a la est celui a changer
	  if (plusOuMoins == 0) {
	    // on fait +
	    if (b == 9) {
	      b = 0 ;
	    } else {
	      b++;
	    }
	  } else {
	    // on fait -
	    if (b == 0) {
	      b = 9 ;
	    } else {
	      b--;
	    }
	  }
	}
	// dans tous les cas on refabrique un nombre de sortie
	nombreSortie += b * diviseur;
  
      }
      
      
      n = n - a*diviseur ;
      diviseur = diviseur / 10 ;
      positionChiffreActuel ++;
    }
  
  
  return nombreSortie ;

}


// Entry point
int main(int argc, char *argv[])
{
  
  licence();
  printf("Version de MAZACAT : %s\n\n", MAZACAT_VERSION);
  printf("Initialisation de SDL...\n");
    // initialisation de SDL (SDL initialization)
    if ( SDL_Init(SDL_INIT_EVERYTHING) < 0 ) 
    {
        fprintf(stderr, "Erreur lors de l'initialisation de SDL: %s\n", SDL_GetError());
        exit(1);
    }


    // Enregistrons que SDL_Quit doit etre appelle lorsque le programme est quitte
    // Ainsi la fermeture de SDL est propre
    // Register SDL_Quit to be called at exit; makes sure things are
    // cleaned up when we quit.
    atexit(SDL_Quit);
    
      
      // Afficher des infos sur les modes video (print some information about video modes)
    printf("Resolution : %u", XRES);
    printf("x%u\n", YRES);
    printf("Profondeurs de couleur a essayer, dans l'ordre :\n");
    printf("Premier mode : %u bits", PREMIERE_PROFONDEUR_COULEUR);
    printf(" (%.0f couleurs)\n", pow(2, PREMIERE_PROFONDEUR_COULEUR));
    printf("Second mode  : %u bits", SECONDE_PROFONDEUR_COULEUR);
    printf(" (%0.f couleurs)\n", pow(2, SECONDE_PROFONDEUR_COULEUR));
    
    // Conseils (print some advice : "you should choose PREMIERE_PROFONDEUR_COULEUR as system ProfondeurCouleur")
    printf("\nRemarque : Pour optenir un affichage optimal, reglez la profondeur de couleur\nde votre systeme sur");
    printf(" %u bits", PREMIERE_PROFONDEUR_COULEUR);
    printf(" (%0.f couleurs)\n", pow(2, PREMIERE_PROFONDEUR_COULEUR));
    printf("(Cette remarque s'affiche meme si votre profondeur de couleur est bonne)\n\n");
    
    
    // 1er essai avec PREMIERE_PROFONDEUR_COULEUR (1st trial with PREMIERE_PROFONDEUR_COULEUR (First Color Depth))
    ProfondeurCouleur = PREMIERE_PROFONDEUR_COULEUR ;
    
    printf("Initialisation de l'affichage - Premier mode : ");
    printf("%u", XRES);
    printf("x%u", YRES);
    printf(" %u bits", ProfondeurCouleur);
    printf(" (%0.f couleurs)\n", pow(2, ProfondeurCouleur));
    
    SDL_WM_SetCaption("MAZACAT", "");
    screen=SDL_SetVideoMode(XRES, YRES, ProfondeurCouleur, AFFICHAGE);
    
    // Si le premier essai echoue, alors on essaye SECONDE_PROFONDEUR_COULEUR (If we fail, try the SECONDE_PROFONDEUR_COULEUR)
    if ( screen == NULL ) 
      {
	
	ProfondeurCouleur = SECONDE_PROFONDEUR_COULEUR ;
	
	printf("Erreur d'initialisation avec le premier mode : \n%s\n", SDL_GetError());
	printf("Initialisation de l'affichage - Second mode : ");
	printf("%u", XRES);
	printf("x%u", YRES);
	printf(" %u bits", ProfondeurCouleur);
	printf(" (%0.f couleurs)\n", pow(2, ProfondeurCouleur));
	
	SDL_WM_SetCaption("MAZACAT", "");
	screen=SDL_SetVideoMode(XRES, YRES, ProfondeurCouleur, AFFICHAGE);
	
      }
    
    // Si on echoue encore, affichier l'erreur et quitter avec code 1 (If we still fail, return error.)
    if ( screen == NULL ) 
      {
        fprintf(stderr, "Erreur lors de l'initialisation de l'affichage : \n%s\n", SDL_GetError());
        exit(1);
      }
    
    
    // on cache la souris car elle est inutile dans les menus
    SDL_ShowCursor(0);
    
    
    printf("Demarrage de MAZACAT...\n");
    
    // Preparation du systeme de scenes (scene system preparing)
    // Choix de la scene initiale (inital scene choice)
    unsigned int scene ;
    scene = 10 ;

    // Quelques declarations : (some declarations :)
    SDL_Event evenement ;
    // passerSceneSuivante est egale a 1 quand on doit passer a la scene suivante
    unsigned short passerSceneSuivante ;

    
    // === SYSTEME DE SCENES ===
    
    while (scene != 0) {
      printf("Scene : %u\n", scene);
      switch (scene) {






	// scene 10 (scene # 10)
      case 10 :
	{
	  // remettre a zero certaines variables
	  niv = 1 ;
	  grp = 1 ;
	  controleAnimal1 = 2 ;






	  dessinerImageVR(screen, "titre.png", 0, 0) ;
	  passerSceneSuivante = 0 ;
	  while (passerSceneSuivante == 0) {
	    unsigned int tick = SDL_GetTicks();
	    if (tick <= dernierTick) 
	      {
		SDL_Delay(1);
	      } else {
		while (SDL_PollEvent(&evenement)) 
		  {
		    switch (evenement.type) 
		      {
		      case SDL_KEYUP:
			break;
		      case SDL_KEYDOWN:
			switch (evenement.key.keysym.sym) {
			case SDLK_ESCAPE :
			  scene = 0;
			  passerSceneSuivante = 1 ;
			  break ;
			case SDLK_RETURN :
			case SDLK_SPACE :
			  scene = 11;
			  passerSceneSuivante = 1 ;
			  break;
			}
			break;
		      case SDL_QUIT:
			fin();
			break;
		      }
		  }
		dernierTick = tick + IMAGE_TOUTES_LES_N_MS;
	      }
	  }
	  break ;
	}
	
	
	
	
	
	
	
      case 11 :
	{
	  dessinerImageVR(screen, "titre2.png", 0, 0) ;
	  passerSceneSuivante = 0 ;
	  while (passerSceneSuivante == 0) {
	    unsigned int tick = SDL_GetTicks();
	    if (tick <= dernierTick) 
	      {
		SDL_Delay(1);
	      } else {
		while (SDL_PollEvent(&evenement)) 
		  {
		    switch (evenement.type) 
		      {
		      case SDL_KEYUP:
			break;
		      case SDL_KEYDOWN:
			switch (evenement.key.keysym.sym) {
			case SDLK_ESCAPE :
			  scene = 0;
			  passerSceneSuivante = 1 ;
			  break ;
			case SDLK_RETURN :
			case SDLK_SPACE :
			  scene = 20;
			  passerSceneSuivante = 1 ;
			  break;
			}
			break;
		      case SDL_QUIT:
			fin();
			break;
		      }
		  }
		dernierTick = tick + IMAGE_TOUTES_LES_N_MS;
	      }
	    
	  }
	  break ;
	}
	
	
	
	
	
	
	
      case 20 :
	{
	  dessinerImageVR(screen, "licence.png", 0, 0) ;

	  SDL_Surface *chiffres30Surface = IMG_Load("chiff30.png");
	  if ( !chiffres30Surface )
	    {
	      printf ( "IMG_Load: %s\n", IMG_GetError () );
	      return 1;
	    }


	  dessinerVersion(chiffres30Surface);
	  passerSceneSuivante = 0 ;
	  while (passerSceneSuivante == 0) {
	    unsigned int tick = SDL_GetTicks();
	    if (tick <= dernierTick) 
	      {
		SDL_Delay(1);
	      } else {
		while (SDL_PollEvent(&evenement)) 
		  {
		    switch (evenement.type) 
		      {
		      case SDL_KEYUP:
			break;
		      case SDL_KEYDOWN:
			switch (evenement.key.keysym.sym) {
			case SDLK_ESCAPE :
			  scene = 0;
			  passerSceneSuivante = 1 ;
			  break ;
			case SDLK_RETURN :
			case SDLK_SPACE :
			  scene = 40;
			  passerSceneSuivante = 1 ;
			  break;
			}
			break;
		      case SDL_QUIT:
			fin();
			break;
		      }
		  }
		dernierTick = tick + IMAGE_TOUTES_LES_N_MS;
	      }
	  }
	  break ;
	}
	
	



	
      case 40 : // menu
	{
	  
	  char imagesFichier[13];
	  signed short choix ;
	  choix = 0 ;
	  
	  strcpy(imagesFichier, "menu.png");
	  SDL_Surface *menuSurface = IMG_Load(imagesFichier);
	  if ( !menuSurface )
	    {
	      printf ( "IMG_Load: %s\n", IMG_GetError () );
	      return 1;
	    }
	  
	  
	  strcpy(imagesFichier, "menuchoi.png");
	  SDL_Surface *menuChoixSurface = IMG_Load(imagesFichier);
	  if ( !menuChoixSurface )
	    {
	      printf ( "IMG_Load: %s\n", IMG_GetError () );
	      return 1;
	    }
	  
	  
	  
	  strcpy(imagesFichier, "menusel.png");
	  SDL_Surface *menuSelecteurSurface = IMG_Load(imagesFichier);
	  if ( !menuSelecteurSurface )
	    {
	      printf ( "IMG_Load: %s\n", IMG_GetError () );
	      return 1;
	    }


	  
	  strcpy(imagesFichier, "chczqsd.png");
	  SDL_Surface *chcZqsdSurface = IMG_Load(imagesFichier);
	  if ( !chcZqsdSurface )
	    {
	      printf ( "IMG_Load: %s\n", IMG_GetError () );
	      return 1;
	    }

	  strcpy(imagesFichier, "chcwasd.png");
	  SDL_Surface *chcWasdSurface = IMG_Load(imagesFichier);
	  if ( !chcWasdSurface )
	    {
	      printf ( "IMG_Load: %s\n", IMG_GetError () );
	      return 1;
	    }

	  strcpy(imagesFichier, "chcsouri.png");
	  SDL_Surface *chcSouriSurface = IMG_Load(imagesFichier);
	  if ( !chcSouriSurface )
	    {
	      printf ( "IMG_Load: %s\n", IMG_GetError () );
	      return 1;
	    }
	  
	    
	    
	  passerSceneSuivante = 0 ;
	  
	  // afficher le menu
	  dessinerMenu(menuSurface, menuSelecteurSurface, menuChoixSurface, choix, chcZqsdSurface, chcWasdSurface, chcSouriSurface);
	  
	  while (passerSceneSuivante == 0) {
	    unsigned int tick = SDL_GetTicks();
	    if (tick <= dernierTick) 
	      {
		SDL_Delay(1);
	      } else {
		
		// detection des touches
		while (SDL_PollEvent(&evenement)) 
		  {
		    switch (evenement.type) 
		      {
		      case SDL_KEYUP:
			break;
		      case SDL_KEYDOWN:
			switch (evenement.key.keysym.sym) {
			case SDLK_ESCAPE :
			  scene = 0;
			  passerSceneSuivante = 1 ;
			  break ;
			case SDLK_DOWN :
			  choix ++ ;
			  if (choix > 7) choix = 0 ;
			  dessinerMenu(menuSurface, menuSelecteurSurface, menuChoixSurface, choix, chcZqsdSurface, chcWasdSurface, chcSouriSurface);
			  break;
			case SDLK_UP :
			  choix -- ;
			  if (choix < 0) choix = 7 ;
			  dessinerMenu(menuSurface, menuSelecteurSurface, menuChoixSurface, choix, chcZqsdSurface, chcWasdSurface, chcSouriSurface);
			  break;
			case SDLK_RETURN :
			case SDLK_SPACE :
			  
			  switch (choix)
			    {
			    case 0 :
			      scene = 41 ; // commencer la partie
			      passerSceneSuivante = 1;
			      break;
			    case 1 :
			      SDL_CD *cdrom;
			      cdrom = SDL_CDOpen(0);
			      if(CD_INDRIVE(SDL_CDStatus(cdrom))) {
				SDL_CDPlayTracks(cdrom, 0, 0, 0, 0);
			      }
			      break;
			    case 2 :
			      if (controleAnimal1 >= 2) {
				controleAnimal1 = 0;
			      } else {
				controleAnimal1++;
			      }
			      dessinerMenu(menuSurface, menuSelecteurSurface, menuChoixSurface, choix, chcZqsdSurface, chcWasdSurface, chcSouriSurface);
			      break;
			      /*
			    case 3 :
			      scene = 3000 ;
			      passerSceneSuivante = 1;
			      break;
			    case 4 :
			      scene = 4000 ;
			      passerSceneSuivante = 1;
			      break;
			    case 5 :
			      scene = 5000 ;
			      passerSceneSuivante = 1;
			      break;
			      */
			    case 6 :
			      passerSceneSuivante = 1;
			      scene = 10 ; // redemarrer
			      break;
			    case 7 :
			      passerSceneSuivante = 1;
			      scene = 0 ; // quitter
			      break;
			    }

			  
			}
			break;
		      case SDL_QUIT:
			fin();
			break;
		      }
		  }
		dernierTick = tick + IMAGE_TOUTES_LES_N_MS;
	      }
	  }
	  break ;
	}
	
	
	
	
      case 41 : // choix du groupe de niveaux
	{


	  niv = 1 ;
	  
	  char imagesFichier[13];

	  strcpy(imagesFichier, "chgrp.png");
	  SDL_Surface *menuSurface = IMG_Load(imagesFichier);
	  if ( !menuSurface )
	    {
	      printf ( "IMG_Load: %s\n", IMG_GetError () );
	      return 1;
	    }


	  strcpy(imagesFichier, "chiffres.png");
	  SDL_Surface *chiffresSurface = IMG_Load(imagesFichier);
	  if ( !chiffresSurface )
	    {
	      printf ( "IMG_Load: %s\n", IMG_GetError () );
	      return 1;
	    }


	  strcpy(imagesFichier, "selhaut.png");
	  SDL_Surface *selecteurSurface = IMG_Load(imagesFichier);
	  if ( !selecteurSurface )
	    {
	      printf ( "IMG_Load: %s\n", IMG_GetError () );
	      return 1;
	    }



	  passerSceneSuivante = 0 ;

	  unsigned short sel;
	  sel = 7 ;
	  
	  // afficher le menu
	  dessinerChoixGroupe(menuSurface, chiffresSurface, grp, selecteurSurface, sel);

	  while (passerSceneSuivante == 0) {
	    unsigned int tick = SDL_GetTicks();
	    if (tick <= dernierTick) 
	      {
		SDL_Delay(1);
	      } else {

		// detection des touches
		while (SDL_PollEvent(&evenement)) 
		  {
		    switch (evenement.type) 
		      {
		      case SDL_KEYUP:
			break;
		      case SDL_KEYDOWN:
			switch (evenement.key.keysym.sym) {
			case SDLK_ESCAPE :
			  scene = 40;
			  passerSceneSuivante = 1 ;
			  break ;
			case SDLK_DOWN :
			  grp = changerUnChiffreDUnNombreDe8Chiffres(grp, sel, 1);
			  dessinerChoixGroupe(menuSurface, chiffresSurface, grp, selecteurSurface, sel);
			  break;
			case SDLK_UP :
			  grp = changerUnChiffreDUnNombreDe8Chiffres(grp, sel, 0);
			  dessinerChoixGroupe(menuSurface, chiffresSurface, grp, selecteurSurface, sel);
			  break;
			case SDLK_RIGHT :
			  if (sel < 7)
			    {
			      sel++;
			      dessinerChoixGroupe(menuSurface, chiffresSurface, grp, selecteurSurface, sel);
			    }
			  break;

			case SDLK_LEFT :
			  if (sel > 0)
			    {
			      sel--;
			      dessinerChoixGroupe(menuSurface, chiffresSurface, grp, selecteurSurface, sel);
			    }
			  break;


			case SDLK_RETURN :
			case SDLK_SPACE :
			  niv = 1 ;
			  scene = 42 ;
			  passerSceneSuivante = 1;
			break;
			}
			break;
		      case SDL_QUIT:
			fin();
			break;
		      }
		  }
		dernierTick = tick + IMAGE_TOUTES_LES_N_MS;
	      }
	  }
	  break ;
	}
	
		

	
      case 42 : // verification du niveau
	{


	  passerSceneSuivante = 0 ;




	  char imagesFichier[13];

	  dessinerImageVR(screen, "verifgrp.png", 0, 0);
	  
	  niv = 1 ;
	  bool niveauOK ;
	  
	  
	  nivgrp2nomsFichiers();
	  // on teste si le fichier de niveau existe
	  FILE *f = fopen(niveauFichier, "rb");	  
	  if (f == NULL) {
	    niveauOK = 0 ;
	    dessinerImageVR(screen, "grperr.png", 0, 0);
	  } else {
	    niveauOK = 1 ;
	    dessinerImageVR(screen, "grpok.png", 0, 0);
	    fclose(f);
	  }
	  
	  
	  
	  
	  while (passerSceneSuivante == 0) {
	    unsigned int tick = SDL_GetTicks();
	    if (tick <= dernierTick) 
	      {
		SDL_Delay(1);
	      } else {

		// detection des touches
		while (SDL_PollEvent(&evenement)) 
		  {
		    switch (evenement.type) 
		      {
		      case SDL_KEYUP:
			break;
		      case SDL_KEYDOWN:
			switch (evenement.key.keysym.sym) {
			case SDLK_ESCAPE :
			  scene = 41;
			  passerSceneSuivante = 1 ;
			  break ;
			case SDLK_RETURN :
			case SDLK_SPACE :
			  if (niveauOK == 1) {
			  scene = 100;
			  passerSceneSuivante = 1 ;
			  }
			break;
			}
			break;
		      case SDL_QUIT:
			fin();
			break;
		      }
		  }
		dernierTick = tick + IMAGE_TOUTES_LES_N_MS;
	      }
	  }
	  break ;
	}
	
		
	
	
      case 100 :
	{
	  dessinerImageVR(screen, "charge.png", 0, 0) ;
	  
	  printf("Chargement des parametres...\n");
	  // declarations
	  unsigned short animaux ; // animaux means animals
	  animaux = 3 ;
	  /*
	    Animaux :   (Animals)
	    0 : souris  (mouse)
	    1 : chat    (cat)
	    2 : chien   (dog)
	  */
	  unsigned short animalActuel ; // animalActuel means current animal
	  unsigned int xCase[animaux] ;
	  xCase[0] = 35 ;
	  xCase[1] = 10 ;
	  xCase[2] = 30 ;
	  unsigned int yCase[animaux] ;
	  yCase[0] = 10 ;
	  yCase[1] = 10 ;
	  yCase[2] = 10 ;
	  unsigned int xVisuel[animaux] ;
	  unsigned int yVisuel[animaux] ;
	  unsigned int decal ; // decalage : px/physicFPS
	  decal = 10 ;
	  unsigned int xMaze ;
	  unsigned int yMaze ;
	  xMaze = 40 ;
	  yMaze = 29 ;
	  
	  unsigned int maze[xMaze][yMaze]  ;
	  unsigned int a;
	  unsigned int b;
	  a = 0;
	  b = 0;

	  bool touchesEnfoncees[nombreTouchesMax] ;
	  unsigned int toucheActuelle ;

	  char imageAnimal[animaux][13] ;
	  strcpy (imageAnimal[0], "souris.png");
	  strcpy (imageAnimal[1], "chat.png");
	  strcpy (imageAnimal[2], "chien.png");


	  unsigned int animalHAUT[animaux] ; // haut
	  unsigned int animalGAUCHE[animaux] ; // gauche
	  unsigned int animalBAS[animaux] ; // bas
	  unsigned int animalDROITE[animaux] ; // droite


	  int xMouse[1];
	  int yMouse[1];



          // animal 0
          animalHAUT[0] = 1 ;
          animalGAUCHE[0] = 2 ;
          animalBAS[0] = 3 ;
          animalDROITE[0] = 4 ;

	  //animal 1
	  switch (controleAnimal1) {
	  case 0:
	    animalHAUT[1] = 126 ;
	    animalGAUCHE[1] = 117 ;
	    animalBAS[1] = 119 ;
	    animalDROITE[1] = 104 ;
	    break;
	  case 1:
	    animalHAUT[1] = 123 ;
	    animalGAUCHE[1] = 101 ;
	    animalBAS[1] = 119 ;
	    animalDROITE[1] = 104 ;
	    break;
	  case 2:
	    animalHAUT[1] = 0 ;
	    animalGAUCHE[1] = 0 ;
	    animalBAS[1] = 0 ;
	    animalDROITE[1] = 0 ;
	    break;
	  default :
	    fprintf(stderr, "Erreur : la variable controlAnimal2 a une valeur inattendue.\n");
	    exit(1);
	  }


          // animal 2
          animalHAUT[2] = 109 ;
          animalGAUCHE[2] = 110 ;
          animalBAS[2] = 111 ;
          animalDROITE[2] = 112 ;






	  SDL_Surface *surfaceAnimal[animaux];

	  // Maintenant nous commencons a preparer le labyrinthe
	  //                  (So now we start preparing the maze)
	  printf("Preparation du labyrinthe : \n");
	  // chargement du fichier du niveau (loading level/maze from file)


	  nivgrp2nomsFichiers(); // conversion des numero de niveau
	  //                         en noms de fichiers

	  printf(" - Chargement du niveau : ");
	  printf(niveauFichier);
	  printf(" ...\n");
	  FILE *f = fopen(niveauFichier, "rb");	  
	  if (f == NULL) {
	    fprintf(stderr, "Fichier introuvable : ");
	    fprintf(stderr, niveauFichier);
	    fprintf(stderr, "\n");
	    exit(1) ;
	  }
	  
	  a = 0;
	  b = 0;
	  while (b<yMaze && !feof(f)) {
	    a = 0;
	    while (a<xMaze && !feof(f)) {
	      maze[a][b] = getc(f);
	      // debug : vous pouvez decommenter la ligne suivante (debug: you may uncomment the following line)
	      //printf("%X,", maze[a][b]);
	      a++;
	    }
	    b++;
	  }
	  
	  fclose(f) ;


	  
	  // mise en place de blocks (0x00) sur les bords (putting blocks (0x00) on borders)
	  // pour etre sur que les animaux ne puissent pas sortir de l'ecran (to make sur animals cannot leave maze)
	  printf(" - Mise en place des bords du labyrinthe...\n");

	  // bord de gauche
	  a = 0;
	  while (a<yMaze) {
	    maze[0][a] = 0x00 ;
	    a++;
	  }
	  // bord de droite
	  
	  a = 0;
	  while (a<yMaze) {
	    maze[39][a] = 0x00 ;
	    a++;
	  }
	  
	  // bord du haut
	  a = 0;
	  while (a<xMaze) {
	    maze[a][0] = 0x00 ;
	    a++;
	  }	  
	
	  // bord du bas
	  
	  a = 0;
	  while (a<xMaze) {
	    maze[a][28] = 0x00 ;
	    a++;
	  }	  
	  
	  

	  a = 0 ;
	  b = 0 ;


	  // calcul des positions visuelles initiales des animaux
	  animalActuel = 0;
	  while (animalActuel < animaux) {
	    xVisuel[animalActuel] = xCase[animalActuel]*20;
	    yVisuel[animalActuel] = yCase[animalActuel]*20;
	    animalActuel++;
	  }
		

	  
	  printf(" - Chargement des images du labyrinthe : ");
	  printf(imagesMazeFichier);
	  printf(" ...\n");
	  SDL_Surface *imagesSurface = IMG_Load(imagesMazeFichier);
	  if ( !imagesSurface )
	    {
	      printf ( "IMG_Load: %s\n", IMG_GetError () );
	      return 1;
	    }

	  imagesCasesSurface = SDL_DisplayFormat(imagesSurface);
	  SDL_FreeSurface(imagesSurface);

	  // loading the image that contains the MAZAanimalActuelT word
	  //(to be under maze)  to mazeSurfaceMC
	  printf(" - Chargement de l'image de fond : pendant.png ...\n");

	  SDL_Surface *mazeSurfaceMauvaisesCouleurs = IMG_Load("pendant.png");
	  if ( !mazeSurfaceMauvaisesCouleurs )
	    {
	      printf ( "IMG_Load: %s\n", IMG_GetError () );
	      return 1;
	    }
	  

	  printf(" - Fabrication de l'image du labyrinthe : \n");
	  printf("    - Conversion de l'image de fond...\n");
	  // nous convertissons mazeSurfaceMauvaisesCouleurs pour avoir le meme format que l'affichage
	  //               (we convert mazeSurfaceMauvaisesCouleurs to have the same format as the Display)
	  // et nous envoyons le resultat vers mazeSurface
	  //               (and we send the result to mazeSurface)
	  mazeSurface = SDL_DisplayFormat(mazeSurfaceMauvaisesCouleurs) ;


	  // Verrouiller la surface si besoin (Lock surface if needed)
	  if (SDL_MUSTLOCK(mazeSurface))
	    if (SDL_LockSurface(mazeSurface) < 0) 
	      return 0;

	  // on dessine les cases vers la mazeSurface
	  //                (drawing the tiles to the mazeSurface)
	  printf("    - Ecriture des cases...\n");
	  a = 0;
	  b = 0;
	  while (b<yMaze) {
	    a = 0;
	    while (a<xMaze) {
	      // A decommenter pour debuggage : (uncomment for debug)
	      //printf("Dessine la case (drawing tile) : %d, %d, %d\n", a * 20, b * 20, maze[a][b]);
	      dessinerCase(a * 20, b * 20, maze[a][b], mazeSurface);
	      a++;
	    }
	    b++;
	  }
	  
	  
	  // Deverrouiller si besoin (Unlock if needed)
	  if (SDL_MUSTLOCK(mazeSurface)) 
	    SDL_UnlockSurface(mazeSurface);
	  
	  
	  // Rafraichir (Tell SDL to update the whole surface)
	  SDL_UpdateRect(mazeSurface, 0, 0, 0, 0);  
	  
	  // declaration
	  Uint8 *etatTouches ;





	  // reading the animal images
	  printf(" - Chargement des images des animaux :\n");
	  animalActuel = 0;
	  while (animalActuel < animaux ) {
	    printf("    - Chargement de l'image : ");
	    printf(imageAnimal[animalActuel]);
	    printf(" ...\n");
	    
	  
	    surfaceAnimal[animalActuel] = IMG_Load(imageAnimal[animalActuel]);
	  if ( !surfaceAnimal[animalActuel] )
	    {
	      printf ( "IMG_Load: %s\n", IMG_GetError () );
	      return 1;
	    }
	  

	  animalActuel ++ ;
	  }
	  
	  if (controleAnimal1 == 2) {
	  printf(" - Activation de la souris pour le controle du chat...\n");
	  SDL_ShowCursor(0);
	  SDL_WM_GrabInput(SDL_GRAB_ON);


	  }
	  
	  
	  // game loop start
	  printf("Demarrage de la partie...\n");

	  	  
	  // Poll for events, and handle the ones we care about.
	  passerSceneSuivante = 0 ;
	  while (passerSceneSuivante == 0) {
	    // Ask SDL for the time in milliseconds	    
	    tick = SDL_GetTicks();
	    
	    
	    
	    if (tick <= dernierTick) 
	      {
		//printf("p");
		SDL_Delay(1);
		//return(0);
	      } else {


		// Zone de calcul et d'affichage central, en gros,
		//les choses a pour affiche chaque image (game core)
		


		//printf("P %u\n", tick);
		
		
		// events not related to player :
		// ESCAPE key, TERM signal
		while (SDL_PollEvent(&evenement)) 
		  {
		    if (evenement.type == SDL_QUIT) 
		      {
			printf("Vous voulez quitter, alors au revoir !\n");
			return(0);
		      }
		  }
		

		// detection des touches (keys detection)
		etatTouches = SDL_GetKeyState(NULL);
		
		// touches generales

		if ( etatTouches[SDLK_ESCAPE] ) {
		  scene = 40;
		  passerSceneSuivante = 1 ;
		}

		// avant de detecter les touches permettant de deplacer
		// l'animal, on regarde si il y a un gagnant
		//      (before detecting keys allowing the player to move
		//       we check if there is a winner)

		// le chien mange le chat
		//           (the dog eats the cat)
		if ( (abs((signed int)(xVisuel[2]-xVisuel[1])) < 20)  && (abs((signed int)(yVisuel[2]-yVisuel[1]))<20) ) {
		  gagnant = 0 ; // le clan souris/chien gagne
		  //                    (group mouse/dog wins)
		  scene = 200 ;
		  passerSceneSuivante = 1 ;

		}


		// le chat mange la souris
		//           (the cat eats the mouse)
		if ( (abs((signed int)(xVisuel[1]-xVisuel[0])) < 20)  && (abs((signed int)(yVisuel[1]-yVisuel[0]))<20) ) {
		  gagnant = 1 ; // le chat gagne (the cat wins)
		  scene = 200 ;
		  passerSceneSuivante = 1 ;
		}

		
		// le chat mange la souris est est mage 
		// par le chien en meme temps
		//           (the cat eats the mouse and is eaten by the dog
		//            at the same time)
		if ( ( (abs((signed int)(xVisuel[1]-xVisuel[0])) < 20)  && (abs((signed int)(yVisuel[1]-yVisuel[0]))<20) ) && ( (abs((signed int)(xVisuel[2]-xVisuel[1])) < 20)  && (abs((signed int)(yVisuel[2]-yVisuel[1]))<20) ) ) {
		  gagnant = -1 ; // personne ne gagne (nobody wins)
		  scene = 200 ;
		  passerSceneSuivante = 1 ;
		}
		

		if (passerSceneSuivante == 1) break;
		
		// Detection des touches appuyees et traduction 
		//                  (Key detection and translation)
		
		// remise a 0 de toutes les touches (resetting all keys to 0)
		toucheActuelle = 0 ;
		while (toucheActuelle < nombreTouchesMax) {
		  touchesEnfoncees[toucheActuelle] = 0 ;
		  toucheActuelle ++ ;
		}

		// touches de direction (direction keys)
		if ( etatTouches[SDLK_UP] )  touchesEnfoncees[1] = 1;
		if ( etatTouches[SDLK_LEFT] )  touchesEnfoncees[2] = 1;
		if ( etatTouches[SDLK_DOWN] )  touchesEnfoncees[3] = 1;
		if ( etatTouches[SDLK_RIGHT] )  touchesEnfoncees[4] = 1;
		// touches avec lettres (letter keys)
		if ( etatTouches[SDLK_a] )  touchesEnfoncees[101] = 1;
		if ( etatTouches[SDLK_b] )  touchesEnfoncees[102] = 1;
		if ( etatTouches[SDLK_c] )  touchesEnfoncees[103] = 1;
		if ( etatTouches[SDLK_d] )  touchesEnfoncees[104] = 1;
		if ( etatTouches[SDLK_e] )  touchesEnfoncees[105] = 1;
		if ( etatTouches[SDLK_f] )  touchesEnfoncees[106] = 1;
		if ( etatTouches[SDLK_g] )  touchesEnfoncees[107] = 1;
		if ( etatTouches[SDLK_h] )  touchesEnfoncees[108] = 1;
		if ( etatTouches[SDLK_i] )  touchesEnfoncees[109] = 1;
		if ( etatTouches[SDLK_j] )  touchesEnfoncees[110] = 1;
		if ( etatTouches[SDLK_k] )  touchesEnfoncees[111] = 1;
		if ( etatTouches[SDLK_l] )  touchesEnfoncees[112] = 1;
		if ( etatTouches[SDLK_m] )  touchesEnfoncees[113] = 1;
		if ( etatTouches[SDLK_n] )  touchesEnfoncees[114] = 1;
		if ( etatTouches[SDLK_o] )  touchesEnfoncees[115] = 1;
		if ( etatTouches[SDLK_p] )  touchesEnfoncees[116] = 1;
		if ( etatTouches[SDLK_q] )  touchesEnfoncees[117] = 1;
		if ( etatTouches[SDLK_r] )  touchesEnfoncees[118] = 1;
		if ( etatTouches[SDLK_s] )  touchesEnfoncees[119] = 1;
		if ( etatTouches[SDLK_t] )  touchesEnfoncees[120] = 1;
		if ( etatTouches[SDLK_u] )  touchesEnfoncees[121] = 1;
		if ( etatTouches[SDLK_v] )  touchesEnfoncees[122] = 1;
		if ( etatTouches[SDLK_w] )  touchesEnfoncees[123] = 1;
		if ( etatTouches[SDLK_x] )  touchesEnfoncees[124] = 1;
		if ( etatTouches[SDLK_y] )  touchesEnfoncees[125] = 1;
		if ( etatTouches[SDLK_z] )  touchesEnfoncees[126] = 1;

		

		// d'abord : le fond : mazeSurface (first, we put the background : mazeSurface)
		SDL_BlitSurface(mazeSurface, NULL, screen, NULL);

		
		// Deplacement des animaux (animal moving)
		animalActuel = 0;
		while (animalActuel < animaux )
		{

		  // DEBUG
		  /*
		  printf("animal : %u", animalActuel);
		  printf("  xCase : %u", xCase[animalActuel]);
		  printf("  yCase : %u", yCase[animalActuel]);
		  printf("  xVisuel : %u", xVisuel[animalActuel]);
		  printf("  yVisuel : %u", yVisuel[animalActuel]);
		  printf("\n");
		  */

		  if (!((xCase[animalActuel]*20 != xVisuel[animalActuel]) || (yCase[animalActuel]*20 != yVisuel[animalActuel]))) {
		    // CAS : Animal sur une case -> detecter les touches
		    if (animalActuel != 1 || controleAnimal1 != 2) {
		      // CAS : on a un animal != chat ou alors c'est le chat
		      // mais il n'est pas controle par la souris (perif)
		      if ((touchesEnfoncees[animalGAUCHE[animalActuel]] == 1) && (maze[xCase[animalActuel] - 1 ][yCase[animalActuel]] != 0 )) xCase[animalActuel]--;
		      if ((touchesEnfoncees[animalDROITE[animalActuel]] == 1) && (maze[xCase[animalActuel] + 1 ][yCase[animalActuel]] != 0 )) xCase[animalActuel]++;
		      if ((touchesEnfoncees[animalHAUT[animalActuel]] == 1) && (maze[xCase[animalActuel]][yCase[animalActuel] - 1 ] != 0 )) yCase[animalActuel]--;
		      if ((touchesEnfoncees[animalBAS[animalActuel]] == 1) && (maze[xCase[animalActuel]][yCase[animalActuel] + 1 ] != 0 )) yCase[animalActuel]++;
		    } else {
		      // si c'est le chat et qu'il est regle sur controle
		      // par souris, alors detecter la souris

		      SDL_GetRelativeMouseState(xMouse, yMouse);
		      
		      //printf("xMouse : %d     yMouse : %d\n", xMouse[0], yMouse[0]);
		      

		      if ((abs(xMouse[0])>abs(yMouse[0]) && xMouse[0]<0) && (maze[xCase[animalActuel] - 1 ][yCase[animalActuel]] != 0 )) xCase[animalActuel]--;
		      if ((abs(xMouse[0])>abs(yMouse[0]) && xMouse[0]>0) && (maze[xCase[animalActuel] + 1 ][yCase[animalActuel]] != 0 )) xCase[animalActuel]++;
		      if ((abs(xMouse[0])<abs(yMouse[0]) && yMouse[0]<0) && (maze[xCase[animalActuel]][yCase[animalActuel] - 1 ] != 0 )) yCase[animalActuel]--;
		      if ((abs(xMouse[0])<abs(yMouse[0]) && yMouse[0]>0) && (maze[xCase[animalActuel]][yCase[animalActuel] + 1 ] != 0 )) yCase[animalActuel]++;

		      /*
		      if ((yMouse[0] > yCase[animalActuel]*20 && yMouse[0] < yCase[animalActuel]*20 + 20 && xMouse[0] < xCase[animalActuel]*20) && (maze[xCase[animalActuel] - 1 ][yCase[animalActuel]] != 0 )) xCase[animalActuel]--;
		      if ((yMouse[0] > yCase[animalActuel]*20 && yMouse[0] < yCase[animalActuel]*20 + 20 && xMouse[0] > xCase[animalActuel]*20 + 20) && (maze[xCase[animalActuel] + 1 ][yCase[animalActuel]] != 0 )) xCase[animalActuel]++;
		      if ((xMouse[0] > xCase[animalActuel]*20 && xMouse[0] < xCase[animalActuel]*20 + 20 && yMouse[0] < yCase[animalActuel]*20) && (maze[xCase[animalActuel]][yCase[animalActuel] - 1 ] != 0 )) yCase[animalActuel]--;
		      if ((xMouse[0] > xCase[animalActuel]*20 && xMouse[0] < xCase[animalActuel]*20 + 20 && yMouse[0] > yCase[animalActuel]*20 + 20) && (maze[xCase[animalActuel]][yCase[animalActuel] + 1 ] != 0 )) yCase[animalActuel]++;
		      */



		    }
		  }	    
		  
		  
		  
		  // CAS : animal entre 2 cases ->
		  // on le deplace (animation de changement de case)
		  if ((xCase[animalActuel]*20 != xVisuel[animalActuel]) || (yCase[animalActuel]*20 != yVisuel[animalActuel])) {
		    if (xCase[animalActuel]*20 < xVisuel[animalActuel]) {
		      xVisuel[animalActuel] -= decal ;
		    } else {
		      if (xCase[animalActuel]*20 > xVisuel[animalActuel]) {
			xVisuel[animalActuel] += decal ;
		      } else {
			if (yCase[animalActuel]*20 < yVisuel[animalActuel]) {
			  yVisuel[animalActuel] -= decal ;
			} else {
			  if (yCase[animalActuel]*20 > yVisuel[animalActuel]) {
			    yVisuel[animalActuel] += decal ;
			  }
			}
		      }
		    }
		  } 
		  animalActuel++;
		}
		  


		// affichage des animaux (displaying the animals)
		animalActuel = 0;
		while (animalActuel < animaux) {
		  dessinerAnimal(surfaceAnimal[animalActuel], screen, xVisuel[animalActuel], yVisuel[animalActuel] );
		  animalActuel++;
		}
		
		

		// Rafrachissement (screen updating)
		SDL_UpdateRect(screen, 0, 0, 0, 0);  


		// fin de la zone de clacul et affichage (END game core)

		dernierTick = tick + IMAGE_TOUTES_LES_N_MS;
	      }
	    
	  }
	  
	  
	  
	  
	  // Fin de la boucle qui troune tant que passerSceneSuivante == 0 (end of the loop which runs while passerSceneSuivante == 0)
	  
	  
	  
	  break ;
	  // fin de la scene 100, ouf ! (END of scene # 100)
	}
	
	
	


	
      case 200 :
	{
	  


	  
	  if (gagnant == 1) {
	    dessinerImageVR(screen, "victchat.png", 0, 0) ;
	  }
	  
	  if (gagnant == 0) {
	    dessinerImageVR(screen, "victsour.png", 0, 0) ;
	  }
	  
	  if (gagnant == -1) {
	    dessinerImageVR(screen, "egalite.png", 0, 0) ;
	  }
	  
	  passerSceneSuivante = 0 ;
	  while (passerSceneSuivante == 0) {
	    unsigned int tick = SDL_GetTicks();
	    if (tick <= dernierTick) 
	      {
		SDL_Delay(1);
	      } else {
		while (SDL_PollEvent(&evenement)) 
		  {
		    switch (evenement.type) 
		      {
		      case SDL_KEYUP:
			break;
		      case SDL_KEYDOWN:
			switch (evenement.key.keysym.sym) {
			case SDLK_ESCAPE :
			  scene = 40;
			  passerSceneSuivante = 1 ;
			  break ;
			case SDLK_RETURN :
			case SDLK_SPACE :
			  passerSceneSuivante = 1 ;
			  if (niv == 99999999)
			    {
			      scene = 40 ;
			    } else {
			      niv++;
			      nivgrp2nomsFichiers();
			      // on teste si le fichier de niveau existe
			      printf("Le niveau %u existe-t-il ? ", niv);
			      
			      FILE *f = fopen(niveauFichier, "rb");	  
			      if (f == NULL) {
				printf("Non. On retourne au menu.\n");
				scene = 40;
			      } else {
				printf("Oui. Passage a ce niveau.\n");
				scene = 100 ;
				fclose(f);
			      }
			      
			      
			    }
			  
			  
			  
			  break;
			}
			break;
		      case SDL_QUIT:
			fin();
			break;
		      }
		  }
		dernierTick = tick + IMAGE_TOUTES_LES_N_MS;
	      }
	    
	  }
	  break ;
	}
	
	
	
	
	
	
	
	
	// En cas de scene incorrecte (IN CASE OF INCORRECT SCENE)
      default :
	{
	  // afficher une erreur avec le numero de la scene introuvable en question (display error with unfindable scene number)
	  fprintf(stderr, "Erreur : scene introuvable : %u\n", scene);
	  // quitte (exits)
	  exit(1) ;
	  break ;
	}
	
      }
    }
    // Sortie du systeme de scenes, quand scene == 0 (exit of scene system, when scene == 0)
    
    SDL_WM_GrabInput(SDL_GRAB_OFF);
    SDL_ShowCursor(1);
    
    printf("Au revoir.\n") ;
    
    
    
}
// Et voila la fin du programme, enfin ! (program end)

