/* Author:  Uyen Truong
 * Copyright (C) 2018, utplay v0.1
 */

#include <assert.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <SDL.h>
#include <SDL_mixer.h>

typedef struct song
{
    char        *title;
    struct song *next;
} Song;

typedef struct playlist
{
    Song *head;
    int   length;
} Playlist;

Song     *new_song(char *title);
Playlist *create_playlist(char *dir_path);
Song     *cherry_pick(Playlist *list, int n);
Playlist *shuffle(Playlist *list);
void      prepend(Playlist *list, Song *song);
void      append(Playlist *list, Song *song);
void      print_list(Playlist *list);
void      list_free(Playlist *list);
void      main_menu(Playlist *list);
void      info(int n);
void      play(Playlist *list);

// TODO: read album dir from cmd line arg
int main(int argc, char ** argv)
{
    srand(time(NULL));
    char *dir_path = argv[1];

    // Create playlist from album
    Playlist *list = create_playlist(dir_path);
    main_menu(list);

    list_free(list);
    return 0;
}

Song *new_song(char *title)
{
    Song *p = malloc(sizeof(struct song));

    p->title = title;
    p->next  = NULL;

    return p;
}

Playlist *create_playlist(char *dir_path)
{
    Playlist *list = malloc(sizeof(struct playlist));
    list->head     = NULL;
    list->length   = 0;

    DIR           *d;
    struct dirent *dir;
    char          *is_mp3;
    char          *path;

    d = opendir(dir_path);

    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            is_mp3 = strstr(dir->d_name, ".mp3");
            if (is_mp3 != NULL)
            {
                path = malloc(strlen(dir_path) + strlen(dir->d_name) + 1);
                strcpy(path, dir_path);
                strcat(path, dir->d_name);
                append(list, new_song(path));
            }
        }
        closedir(d);
    }
    return list;
}

Song *cherry_pick(Playlist *list, int n)
{
    Song *p = list->head;

    assert(p != NULL);
    assert(n >= 0 && n < list->length);

    if (n == 0)
    {
        list->head = p->next;
        list->length--;
        return p;
    }
    else
    {
        for (int i = 0; i < n - 1; i++)
            p = p->next;

        Song *cherry = p->next;
        p->next      = cherry->next;
        list->length--;
        return cherry;
    }
}

void prepend(Playlist *list, Song *song)
{
    song->next = list->head;
    list->head = song;
    list->length++;
}

Playlist *shuffle(Playlist *list)
{
    Playlist *new_list = malloc(sizeof(Playlist));
    new_list->head   = NULL;
    new_list->length = 0;

    while (list->head != NULL)
    {
        int n = rand() % list->length;
        Song *song = cherry_pick(list, n);

        prepend(new_list, song);
    }

    print_list(new_list);
    return new_list;
}


void append(Playlist *list, Song *song)
{
    if (list->head == NULL)
    {
        list->head = song;
        list->length++;
    }
    else
    {
        Song *p = list->head;

        while (p->next != NULL)
            p = p->next;

        p->next = song;
        song->next = NULL;
        list->length++;
    }
}

void print_list(Playlist *list)
{

    if (list->head != NULL)
    {
        printf("This playlist has %d songs\n\n", list->length);
        for (Song *p = list->head; p != NULL; p = p->next)
            printf("-> %.64s\n", p->title + 8);
        printf("\n");
    }
    else
        printf("This directory doesn't contain mp3 files\n");
}

void list_free(Playlist *list)
{
    for (Song *p = list->head; p != NULL; p = list->head)
    {
        list->head = list->head->next;
        free(p->title);
        free(p);
    }
    free(list);
}

void main_menu(Playlist *list)
{
    char cmd[10];
    int  check = 1;

    while (check)
    {
        printf("Help: (p)lay , (h)elp, (v)ersion, "
               "(s)huffle, (l)ist, (q)uit > ");
        scanf("%s", cmd);
        if (cmd[0] == 'q' || cmd[0] == 'Q')
            check = 0;
        else if (cmd[0] == 'h' || cmd[0] == 'H')
            info(0);
        else if (cmd[0] == 'v' || cmd[0] == 'V')
            info(1);
        else if (cmd[0] == 'l' || cmd[0] == 'L')
            print_list(list);
        else if (cmd[0] == 's' || cmd[0] == 'S')
            list = shuffle(list);
        else
            play(list);
    }
    printf("\nThank you for coming! Goodbye!\n");
}

void info(int n)
{
    if (n == 0) // Print help menu
    {
        printf("\nHELP\n"
               "\nFollow these commands to excute your desire action:\n\n"
               "(h)elp   \tShow help menu\n"
               "(v)ersion\tShow the latest version of utplay\n"
               "(l)ist   \tList the song titles of current playlist\n"
               "(s)huffle\tShuffle the current playlist\n"
               "(p)lay   \tStart playing the 1st song from the playlist\n"
               "\t(P)ause \tPause the current playing song\n"
               "\t(R)esume\tResume playing the paused song\n"
               "\t(B)ack  \tBack to the previous song\n"
               "\t(N)ext  \tJump to the next song\n"
               "\t(S)top  \tStop the music player, back to main menu\n"
               "(q)uit   \tQuit the program\n\n");
    }
    else // Print the latest version
    {
        printf("\nutplay v0.1\n"
               "Copyright (C) 2018 Uyen Truong\n"
               "Contact me via haiuyentruong(at)gmail(dot)com if found bugs\n\n");
    }
}

void play(Playlist *list)
{
    Mix_Music *music;
    char       buf[10];
    Song      *prev_song = NULL;
    Song      *song      = list->head;
    int        does_stop = 0;

    // Start SDL with audio support
    if (SDL_Init(SDL_INIT_AUDIO) == -1)
    {
        printf("SDL_Init: %s\n", SDL_GetError());
        exit(1);
    }

    // Open 44.1KHz, signed 16bit, system byte order,
    // stereo audio, using 1024 byte chunks
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) == -1)
    {
        printf("Mix_OpenAudio: %s\n", Mix_GetError());
        exit(2);
    }

    while (song != NULL)
    {
        if (does_stop) // Stop the music player
            break;

        // Load the MP3 file "music.mp3" to play as music
        music = Mix_LoadMUS(song->title);
        if (!music)
            printf("Mix_LoadMUS(): %s\n", Mix_GetError());

        if (Mix_PlayMusic(music, 0) == -1)
            printf("Mix_PlayMusic: %s\n", Mix_GetError());

        printf("\n💿 Playing %.64s\n\n", song->title + 8);
        while (Mix_PlayingMusic())
        {
            int move_back = 0;
            int check = 1;
            SDL_Delay(100);
            while (check)
            {
                printf("Help: (p)ause, (r)esume,"
                       " (b)efore, (n)ext, (s)top > ");
                if (!Mix_PlayingMusic())
                {
                    printf("\n");
                    check = 0;
                }
                else
                {
                    scanf("%s", buf);
                    if (buf[0] == 'p' || buf[0] == 'P')
                        Mix_PauseMusic();
                    else if (buf[0] == 'r' || buf[0] == 'R')
                        Mix_ResumeMusic();
                    else if (buf[0] == 'n' || buf[0] == 'N')
                    {
                        Mix_HaltMusic();
                        check = 0;
                    }
                    else if (buf[0] == 'b' || buf[0] == 'B')
                    {
                        Mix_HaltMusic();
                        song = prev_song;
                        move_back = 1;
                        check = 0;
                    }
                    else if ((buf[0] == 's' || buf[0] == 'S'))
                    {
                        Mix_HaltMusic();
                        does_stop = 1;
                        check = 0;
                    }
                }
            }
            if (!move_back)
            {
                prev_song = song;
                song = song->next;
            }
        }

        Mix_FreeMusic(music);
        music = NULL;
        SDL_Delay(200);
    }

    Mix_CloseAudio();
    SDL_Quit();
}
