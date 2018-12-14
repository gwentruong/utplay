/* Author:  Uyen Truong
   Copyright (C) 2018, utplay v0.1 */

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <SDL.h>
#include <SDL_mixer.h>

typedef struct songs
{
    char *title;
    struct songs *next;
} Songs;

typedef struct playlist
{
    Songs *head;
    int amount;
} Playlist;

Songs    *new_song(char *title);
Playlist *create_playlist(void);
void      info(int n);
void      menu_interact(void);
void      append(Playlist **ptr_list, Songs *song);
void      print_list(Playlist *list);
void      list_free(Playlist *list);

int main(void)
{
    // Create playlist from album
    Playlist *list   = create_playlist();
    // int next_track   = 0;

    print_list(list);


    list_free(list);
    return 0;
}

Songs *new_song(char *title)
{
    Songs *p = malloc(sizeof(struct songs));

    p->title = title;
    p->next  = NULL;

    return p;
}

Playlist *create_playlist(void)
{
    Playlist *list = malloc(sizeof(struct playlist));
    list->head     = NULL;
    list->amount   = 0;

    DIR *d;
    struct dirent *dir;
    char *check_name;
    d = opendir("./album");

    if (d)
    {
        while ((dir = readdir(d)) != NULL)
        {
            check_name = strstr(dir->d_name, ".mp3");
            if (check_name != NULL)
            {
                check_name = malloc(strlen("./album/") + strlen(dir->d_name) + 1);
                strcpy(check_name, "./album/");
                strcat(check_name, dir->d_name);
                append(&list, new_song(check_name));
            }
        }
        closedir(d);
    }
    return list;
}

void append(Playlist **ptr_list, Songs *song)
{
    Playlist *temp = *ptr_list;

    if (temp->head == NULL)
    {
        temp->head = song;
        temp->amount++;
    }
    else
    {
        Songs *p = temp->head;

        while (p->next != NULL)
            p = p->next;

        p->next = song;
        song->next = NULL;
        temp->amount++;
    }
}

void print_list(Playlist *list)
{
    printf("This playlist has %d songs.\n", list->amount);
    if (list->head != NULL)
    {
        for (Songs *p = list->head; p != NULL; p = p->next)
            printf("Song path %s %p\n", p->title, p);
    }
    else
        printf("This playlist is empty.\n");
}

void list_free(Playlist *list)
{
    for (Songs *p = list->head; p != NULL; p = list->head)
    {
        list->head = list->head->next;
        list->amount--;
        free(p->title); // Free check_name
        free(p);
    }
    free(list);
}

void info(int n)
{
    if (n == 0) // Print help menu
    {
        printf("HELP\n"
               "Follow these commands to excute your desire action:\n"
               "help, h \tShow help menu\n"
               "version\tShow the latest version of utplay\n"
               "shuffle\tShuffle the current playlist\n"
               "play\tStart playing the 1st song from the playlist "
               "or choose the title of the song\n"
               "\tstop\tStop playing song\n"
               "\tpause, p\tPause the current playing song\n"
               "\tcontinue, c, resume, r\tResume playing the paused song\n"
               "\tshow\tShow the title of the playing song\n"
               "quit, q\tQuit the program\n");
    }
    else // Print the latest version
    {
        printf("utplay v0.1\n"
               "Copyright (C) 2018 Uyen Truong\n"
               "Contact me via haiuyentruong(at)gmail(dot)com if found bugs\n");
    }
}

void menu_interact(void)
{
    char cmd[10];

    printf("Available commands: (p)ause, (r)esume, (s)top > ");
    fflush(stdin);
    if (scanf("%s", cmd) == 1)
    {
        switch (cmd[0])
        {
            case 'p': case 'P':
                Mix_PauseMusic();
                break;
            case 'r': case 'R':
                Mix_ResumeMusic();
                break;
            case 's': case 'S':
                Mix_HaltMusic();
                break;
        }
    }
}
