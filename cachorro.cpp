//Cachorro minado by Ricardo bustamante de Queiroz

#include <stdlib.h>
#include "SDL/SDL.h"
#include "SDL/SDL_image.h"
#include <string>

/*
legenda:
    bombdisplay:
        X = Desconhecido
        L = Liberado
        F = Bandeira
        ? = Interrogação
    bombmap:
        0~8 = numero de bombas
        9   = bomba
*/

//atributos da tela e outras variaveis
int i,j, k, a, b, x, y, w, i2,j2, cursorx, cursory;
bool exploded;
char bombDisp[100][100];
int bombMap[100][100], bombN=60, lin=10, col=10;
const int BORDER_TOP = 3, BORDER_BOT = 1;
const int BORDER_LEFT = 1, BORDER_RIGHT = 1;
const int SCREEN_WIDTH = (lin+BORDER_LEFT+BORDER_RIGHT)*16;
const int SCREEN_HEIGHT = (col+BORDER_TOP+BORDER_BOT)*16;
const int SCREEN_BPP = 32;

//surfaces
SDL_Surface*tileset = NULL;
SDL_Surface*screen = NULL;

//estrutura de evento
SDL_Event event;

//porções do sprite
SDL_Rect tile[26];

SDL_Surface *load_image( std::string filename ) {
    SDL_Surface* loadedImage = NULL;
    SDL_Surface* optimizedImage = NULL;
    loadedImage = IMG_Load( filename.c_str() );
    if ( loadedImage != NULL ) {
        optimizedImage = SDL_DisplayFormat( loadedImage );
        SDL_FreeSurface( loadedImage );
        if ( optimizedImage != NULL ) {
            SDL_SetColorKey( optimizedImage, SDL_SRCCOLORKEY, SDL_MapRGB( optimizedImage->format, 0xFF, 0x00, 0xFF ) );
        }
    }
    return optimizedImage;
}

void apply_surface( int x, int y, SDL_Surface* source, SDL_Surface* destination, SDL_Rect* clip = NULL ) {
    SDL_Rect offset;
    offset.x = x;
    offset.y = y;
    SDL_BlitSurface( source, clip, destination, &offset );
}

bool init() {
    if ( SDL_Init( SDL_INIT_EVERYTHING ) == -1 ) {
        return false;
    }
    screen = SDL_SetVideoMode( SCREEN_WIDTH, SCREEN_HEIGHT, SCREEN_BPP, SDL_SWSURFACE );
    if ( screen == NULL ) {
        return false;
    }
    SDL_WM_SetCaption( "Cachorro Minado v1.0", NULL );
    return true;
}

bool load_files() {
    //carregando o sprite
    tileset = load_image( "graphics.png" );

    //Se houver algum problema carregando o mapa
    if ( tileset == NULL ) {
        return false;
    }

    //se tudo tiver carregado ok
    return true;
}

void clean_up() {
    //liberar o spritemap
    SDL_FreeSurface( tileset );

    //Sair do sdl
    SDL_Quit();
}

//define os numeros para cada parte da imagem
void set_clips() {
    for (i=0;i<13;i++) {
        tile[ i ].x = 16*i;
        tile[ i ].y = 0;
        tile[ i ].w = 16;
        tile[ i ].h = 16;

        tile[ i+13 ].x = 16*i;
        tile[ i+13 ].y = 16;
        tile[ i+13 ].w = 16;
        tile[ i+13 ].h = 16;
    }
}

void drawBorder() {
    //primeira linha
    apply_surface( 0*16, 0, tileset, screen, &tile[ 13 ] );
    for (i=1;i<=lin;i++) {
        apply_surface( 16*i, 0, tileset, screen, &tile[ 14 ] );
    }
    apply_surface( (lin+1)*16, 0, tileset, screen, &tile[ 15 ] );

    //segunda linha
    apply_surface( 0, 16, tileset, screen, &tile[ 16 ] );
    for (i=1;i<=lin;i++) {
        apply_surface( 16*i, 16, tileset, screen, &tile[ 17 ] );
    }
    apply_surface( (lin+1)*16, 16, tileset, screen, &tile[ 18 ] );
    apply_surface( 2*16, 16, tileset, screen, &tile[ 23 ] );
    apply_surface( (lin-1)*16, 16, tileset, screen, &tile[ 22 ] );

    //terceira linha
    apply_surface( 0, 32, tileset, screen, &tile[ 16 ] );
    for (i=1;i<=lin;i++) {
        apply_surface( 16*i, 32, tileset, screen, &tile[ 17 ] );
    }
    apply_surface( (lin+1)*16, 32, tileset, screen, &tile[ 18 ] );

    //parte lateral
    for (j=2;j<col+BORDER_TOP;j++) {
        apply_surface( 0, j*16, tileset, screen, &tile[ 16 ] );
        apply_surface( (lin+1)*16, j*16, tileset, screen, &tile[ 18 ] );
    }

    //ultima linha
    apply_surface( 0, (col+BORDER_TOP)*16, tileset, screen, &tile[ 19 ] );
    for (i=1;i<=lin;i++) {
        apply_surface( 16*i, (col+BORDER_TOP)*16, tileset, screen, &tile[ 20 ] );
    }
    apply_surface( (lin+1)*16, (col+BORDER_TOP)*16, tileset, screen, &tile[ 21 ] );
}

//função que posiciona as bombas no inicio de cada jogo
void setUpBomb () {
    exploded = false;
    //zerando o mapa, ou seja, não há bombas. 9 é bomba, n é o numero delas.
    for (j=0;j<col;j++) {
        for (i=0;i<lin;i++) {
            bombMap[i][j] = 0;
            bombDisp[i][j] = 'X';
        }
    }
    //sorteando posições das bombas no mapa
    for (k=0; k<bombN;k++) {
        a = rand()%(lin);
        b = rand()%(col);
        while (bombMap[a][b]==9) {
            a = rand()%(lin);
            b = rand()%(col);
        }
        //posição (a,b) sorteada. incrementar contadores de bomba.
        bombMap[a][b]=9;
        for (i=a-1;i<=a+1;i++) {
            for (j=b-1;j<=b+1;j++) {
                if (bombMap[i][j] <= 8) {
                    bombMap[i][j]++;
                }
            }
        }
    }
}

void explodeBombs() {
    for (i=0;i<lin;i++) {
        for (j=0;j<col;j++) {
            if (bombMap[i][j]==9 && bombDisp[i][j]!='B') {
                bombDisp[i][j] = 'L';
            }
        }
    }
}

//função que revela uma area ao redor do local selecionado. Caso o numero
//de bombas reveladas seja zero, verificar novamente até que seja diferente.
void revealArea(int m, int n) {
    if (bombMap[m][n] == 0) {
        bombDisp[m][n] = 'L';
        /*VERSÃO COM FOR BUGADA... VAI ENTENDER... o_O
        for (i=-1;i<=1;i++) {
            for (j=-1;j<=1;j++) {
                if (bombDisp[m+i][n+j]=='?' && m+i>=0 && m+i<lin && n+j<col &&  n+j>=0) {
                    revealArea(m+i,n+j);
                }
            }
        }*/

        //GAMBIARRA MODE *ON*
        i=m-1;
        j=n-1;
        if (bombDisp[i][j]=='X' && i>=0 && i<lin && j<col &&  j>=0) {
            revealArea(i,j);
        }
        i=m-1;
        j=n;
        if (bombDisp[i][j]=='X' && i>=0 && i<lin && j<col &&  j>=0) {
            revealArea(i,j);
        }
        i=m-1;
        j=n+1;
        if (bombDisp[i][j]=='X' && i>=0 && i<lin && j<col &&  j>=0) {
            revealArea(i,j);
        }
        i=m;
        j=n-1;
        if (bombDisp[i][j]=='X' && i>=0 && i<lin && j<col &&  j>=0) {
            revealArea(i,j);
        }
        i=m;
        j=n;
        if (bombDisp[i][j]=='X' && i>=0 && i<lin && j<col &&  j>=0) {
            revealArea(i,j);
        }
        i=m;
        j=n+1;
        if (bombDisp[i][j]=='X' && i>=0 && i<lin && j<col &&  j>=0) {
            revealArea(i,j);
        }
        i=m+1;
        j=n-1;
        if (bombDisp[i][j]=='X' && i>=0 && i<lin && j<col &&  j>=0) {
            revealArea(i,j);
        }
        i=m+1;
        j=n;
        if (bombDisp[i][j]=='X' && i>=0 && i<lin && j<col &&  j>=0) {
            revealArea(i,j);
        }
        i=m+1;
        j=n+1;
        if (bombDisp[i][j]=='X' && i>=0 && i<lin && j<col &&  j>=0) {
            revealArea(i,j);
        }
        //FIM GAMBIARRA MODE... (ou não?)
    } else if (bombMap[m][n] != 9) {
        bombDisp[m][n] = 'L';
    } else {
        bombDisp[m][n] = 'B';
        exploded = true;
        explodeBombs();
        //game over
    }
}

void changeFlag (int m, int n) {
    if (bombDisp[m][n] == 'X') {
        bombDisp[m][n] = 'F';
    } else if (bombDisp[m][n] == 'F') {
        bombDisp[m][n] = '?';
    } else {
        bombDisp[m][n] = 'X';
    }

}

//monitora os eventos
void handle_events() {
    if(exploded == false){
    //Se o botão do mouse for apertado
    if ( event.type == SDL_MOUSEBUTTONDOWN ) {
        if ( event.button.button == SDL_BUTTON_LEFT) {
            //pegar posição do mouse
            x = (event.button.x/16)-BORDER_LEFT;
            y = (event.button.y/16)-BORDER_TOP;
            if (bombDisp[x][y]!='F') {
                revealArea(x,y);
            }
        } else if ( event.button.button == SDL_BUTTON_RIGHT) {
            //pegar posição do mouse
            x = (event.button.x/16)-BORDER_LEFT;
            y = (event.button.y/16)-BORDER_TOP;
            if (bombDisp[x][y]!='L' && bombDisp[x][y]!='B') {
                changeFlag(x,y);
            }
        }
    }
    if ( event.type == SDL_MOUSEMOTION ) {
        cursorx = (event.button.x/16)-BORDER_LEFT;
        cursory = (event.button.y/16)-BORDER_TOP;
    }}
}

//atualiza os sprites de acordo com as matrizes
void update_field() {
    for (i=0;i<lin;i++) {
        for (j=0;j<col;j++) {
            switch (bombDisp[i][j]) {
            case 'B':
                apply_surface( (i+BORDER_LEFT)*16, (j+BORDER_TOP)*16, tileset, screen, &tile[ 12 ] );
                break;
            case 'X':
                apply_surface( (i+BORDER_LEFT)*16, (j+BORDER_TOP)*16, tileset, screen, &tile[ 0 ] );
                break;
            case 'F':
                apply_surface( (i+BORDER_LEFT)*16, (j+BORDER_TOP)*16, tileset, screen, &tile[ 25 ] );
                break;
            case '?':
                apply_surface( (i+BORDER_LEFT)*16, (j+BORDER_TOP)*16, tileset, screen, &tile[ 24 ] );
                break;
            default:
                apply_surface( (i+BORDER_LEFT)*16, (j+BORDER_TOP)*16, tileset, screen, &tile[ bombMap[i][j]+2 ] );
                break;
            }
        }
    }
    if (cursorx>=0 && cursorx<lin && cursory>=0&&cursory<col && bombDisp[cursorx][cursory]=='X') {
        apply_surface((cursorx+BORDER_LEFT)*16, (cursory+BORDER_TOP)*16, tileset, screen, &tile[ 1 ] );
    }
}

int main( int argc, char* args[] ) {
    srand(time(NULL));
    scanf("%d %d %d", &lin,&col,&bombN);
    //Flag de saída
    bool quit = false;
    //inicializar
    if ( init() == false ) {
        return 1;
    }
    //carregar arquivos
    if ( load_files() == false ) {
        return 1;
    }
    //numeração dos tiles
    set_clips();
    //colocar as bombas nos devidos lugares
    setUpBomb();
    //encher a tela de cor
    SDL_FillRect( screen, &screen->clip_rect, SDL_MapRGB( screen->format, 0xFF, 0x00, 0x00 ) );
    //aplicar sprites da moldura à tela
    drawBorder();
    //Enquanto o usuario não sair
    while ( quit == false ) {
        //enquanto existirem eventos a serem executados
        while ( SDL_PollEvent( &event ) ) {
            handle_events();
            //Se o usuario fechar a janela no X

            if ( event.type == SDL_QUIT ) {
                //sair do programa
                quit = true;
            }
        }
        update_field();
        //Atualizar a tela
        if ( SDL_Flip( screen ) == -1 ) {
            return 1;
        }
    }

    //Liberar as imagens e sair do SDL
    clean_up();

    return 0;
}
