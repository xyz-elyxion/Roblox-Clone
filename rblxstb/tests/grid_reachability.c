#define RBLXSTB_CONNECTED_COMPONENTS_IMPLEMENTATION
#define RBLXSTBCC_GRID_COUNT_X_LOG2  10
#define RBLXSTBCC_GRID_COUNT_Y_LOG2  10
#include "rblxstb_connected_components.h"

#ifdef GRID_TEST

#include <windows.h>
#include <stdio.h>
#include <direct.h>

//#define RBLXSTB_DEFINE
#include "rblxstb.h"

//#define RBLXSTB_IMAGE_IMPLEMENTATION
#include "rblxstb_image.h"

//#define RBLXSTB_IMAGE_WRITE_IMPLEMENTATION
#include "rblxstb_image_write.h"

typedef struct
{
   uint16 x,y;
} point;

point leader[1024][1024];
uint32 color[1024][1024];

point find(int x, int y)
{
   point p,q;
   p = leader[y][x];
   if (p.x == x && p.y == y)
      return p;
   q = find(p.x, p.y);
   leader[y][x] = q;
   return q;
}

void onion(int x1, int y1, int x2, int y2)
{
   point p = find(x1,y1);
   point q = find(x2,y2);

   if (p.x == q.x && p.y == q.y)
      return;

   leader[p.y][p.x] = q;
}

void reference(uint8 *map, int w, int h)
{
   int i,j;

   for (j=0; j < h; ++j) {
      for (i=0; i < w; ++i) {
         leader[j][i].x = i;
         leader[j][i].y = j;
      }
   }
         
   for (j=1; j < h-1; ++j) {
      for (i=1; i < w-1; ++i) {
         if (map[j*w+i] == 255) {
            if (map[(j+1)*w+i] == 255) onion(i,j, i,j+1);
            if (map[(j)*w+i+1] == 255) onion(i,j, i+1,j);
         }
      }
   }

   for (j=0; j < h; ++j) {
      for (i=0; i < w; ++i) {
         uint32 c = 0xff000000;
         if (leader[j][i].x == i && leader[j][i].y == j) {
            if (map[j*w+i] == 255)
               c = rblxstb_randLCG() | 0xff404040;
         }
         color[j][i] = c;
      }
   }

   for (j=0; j < h; ++j) {
      for (i=0; i < w; ++i) {
         if (leader[j][i].x != i || leader[j][i].y != j) {
            point p = find(i,j);
            color[j][i] = color[p.y][p.x];
         }
      }
   }
}

void write_map(rblxstbcc_grid *g, int w, int h, char *filename)
{
   int i,j;
   for (j=0; j < h; ++j) {
      for (i=0; i < w; ++i) {
         unsigned int c;
         c = rblxstbcc_get_unique_id(g,i,j);
         c = rblxstb_rehash_improved(c)&0xffffff;
         if (c == RBLXSTBCC_NULL_UNIQUE_ID)
            c = 0xff000000;
         else
            c = (~c)^0x555555;
         if (i % 32 == 0 || j %32 == 0) {
            int r = (c >> 16) & 255;
            int g = (c >> 8) & 255;
            int b = c & 255;
            r = (r+130)/2;
            g = (g+130)/2;
            b = (b+130)/2;
            c = 0xff000000 + (r<<16) + (g<<8) + b;
         }
         color[j][i] = c;
      }
   }
   rblxstbi_write_png(filename, w, h, 4, color, 4*w);
}

void test_connected(rblxstbcc_grid *g)
{
   int n = rblxstbcc_query_grid_node_connection(g, 512, 90, 512, 871);
   //printf("%d ", n);
}

static char *message;
LARGE_INTEGER start;

void start_timer(char *s)
{
   message = s;
   QueryPerformanceCounter(&start);
}

void end_timer(void)
{
   LARGE_INTEGER end, freq;
   double tm;

   QueryPerformanceCounter(&end);
   QueryPerformanceFrequency(&freq);

   tm = (end.QuadPart - start.QuadPart) / (double) freq.QuadPart;
   printf("%6.4lf ms: %s\n", tm * 1000, message);
}

extern void quicktest(void);

int loc[5000][2];
int main(int argc, char **argv)
{
   rblxstbcc_grid *g;

   int w,h, i,j,k=0, count=0, r;
   uint8 *map = rblxstbi_load("data/map_03.png", &w, &h, 0, 1);

   assert(map);
   quicktest();

   for (j=0; j < h; ++j)
      for (i=0; i < w; ++i)
         map[j*w+i] = ~map[j*w+i];

   for (i=0; i < w; ++i)
      for (j=0; j < h; ++j)
         //map[j*w+i] = (((i+1) ^ (j+1)) >> 1) & 1 ? 255 : 0;
         map[j*w+i] = rblxstb_max(abs(i-w/2),abs(j-h/2)) & 1 ? 255 : 0;
         //map[j*w+i] = (((i ^ j) >> 5) ^ (i ^ j)) & 1 ? 255 : 0;
         //map[j*w+i] = rblxstb_rand() & 1 ? 255 : 0;

   #if 1
   for (i=0; i < 100000; ++i)
      map[(rblxstb_rand()%h)*w + rblxstb_rand()%w] ^= 255;
   #endif
            
   _mkdir("tests/output/rblxstbcc");

   rblxstbi_write_png("tests/output/rblxstbcc/reference.png", w, h, 1, map, 0);

   //reference(map, w, h);

   g = malloc(rblxstbcc_grid_sizeof());
   printf("Size: %d\n", rblxstbcc_grid_sizeof());

#if 0
   memset(map, 0, w*h);
   rblxstbcc_init_grid(g, map, w, h);
   {
      int n;
      char **s = rblxstb_stringfile("c:/x/clockwork_update.txt", &n);
      write_map(g, w, h, "tests/output/rblxstbcc/base.png");
      for (i=1; i < n; i += 1) {
         int x,y,t;
         sscanf(s[i], "%d %d %d", &x, &y, &t);
         if (i == 571678)
            write_map(g, w, h, rblxstb_sprintf("tests/output/rblxstbcc/clockwork_good.png", i));
         rblxstbcc_update_grid(g, x, y, t);
         if (i == 571678)
            write_map(g, w, h, rblxstb_sprintf("tests/output/rblxstbcc/clockwork_bad.png", i));
         //if (i > 571648 && i <= 571712)
            //write_map(g, w, h, rblxstb_sprintf("tests/output/rblxstbcc/clockwork_%06d.png", i));
      }
      write_map(g, w, h, rblxstb_sprintf("tests/output/rblxstbcc/clockwork_%06d.png", i-1));
   }
   return 0;
#endif


   start_timer("init");
   rblxstbcc_init_grid(g, map, w, h);
   end_timer();
   //g = rblxstb_file("c:/x/clockwork_path.bin", 0);
   write_map(g, w, h, "tests/output/rblxstbcc/base.png");

   for (i=0; i < 5000;) {
      loc[i][0] = rblxstb_rand() % w;
      loc[i][1] = rblxstb_rand() % h;
      if (rblxstbcc_query_grid_open(g, loc[i][0], loc[i][1]))
         ++i;
   }

   r = 0;
   start_timer("reachable");
   for (i=0; i < 2000; ++i) {
      for (j=0; j < 2000; ++j) {
         int x1 = loc[i][0], y1 = loc[i][1];
         int x2 = loc[2000+j][0], y2 = loc[2000+j][1];
         r += rblxstbcc_query_grid_node_connection(g, x1,y1, x2,y2);
      }
   }
   end_timer();
   printf("%d reachable\n", r);

   printf("Cluster size: %d,%d\n", RBLXSTBCC__CLUSTER_SIZE_X, RBLXSTBCC__CLUSTER_SIZE_Y);

   #if 1
   for (j=0; j < 10; ++j) {
      for (i=0; i < 5000; ++i) {
         loc[i][0] = rblxstb_rand() % w;
         loc[i][1] = rblxstb_rand() % h;
      }
      start_timer("updating 2500");
      for (i=0; i < 2500; ++i) {
         if (rblxstbcc_query_grid_open(g, loc[i][0], loc[i][1]))
            rblxstbcc_update_grid(g, loc[i][0], loc[i][1], 1);
         else
            rblxstbcc_update_grid(g, loc[i][0], loc[i][1], 0);
      }
      end_timer();
      write_map(g, w, h, rblxstb_sprintf("tests/output/rblxstbcc/update_random_%d.png", j*i));
   }
   #endif

   #if 0
   start_timer("removing");
   count = 0;
   for (i=0; i < 1800; ++i) {
      int x,y,a,b;
      x = rblxstb_rand() % (w-32);
      y = rblxstb_rand() % (h-32);
      
      if (i & 1) {
         for (a=0; a < 32; ++a)
            for (b=0; b < 1; ++b)
               if (rblxstbcc_query_grid_open(g, x+a, y+b)) {
                  rblxstbcc_update_grid(g, x+a, y+b, 1);
                  ++count;
               }
      } else {
         for (a=0; a < 1; ++a)
            for (b=0; b < 32; ++b)
               if (rblxstbcc_query_grid_open(g, x+a, y+b)) {
                  rblxstbcc_update_grid(g, x+a, y+b, 1);
                  ++count;
               }
      }

      //if (i % 100 == 0) write_map(g, w, h, rblxstb_sprintf("tests/output/rblxstbcc/open_random_%d.png", i+1));
   }
   end_timer();
   printf("Removed %d grid spaces\n", count);
   write_map(g, w, h, rblxstb_sprintf("tests/output/rblxstbcc/open_random_%d.png", i));


   r = 0;
   start_timer("reachable");
   for (i=0; i < 1000; ++i) {
      for (j=0; j < 1000; ++j) {
         int x1 = loc[i][0], y1 = loc[i][1];
         int x2 = loc[j][0], y2 = loc[j][1];
         r += rblxstbcc_query_grid_node_connection(g, x1,y1, x2,y2);
      }
   }
   end_timer();
   printf("%d reachable\n", r);

   start_timer("adding");
   count = 0;
   for (i=0; i < 1800; ++i) {
      int x,y,a,b;
      x = rblxstb_rand() % (w-32);
      y = rblxstb_rand() % (h-32);

      if (i & 1) {
         for (a=0; a < 32; ++a)
            for (b=0; b < 1; ++b)
               if (!rblxstbcc_query_grid_open(g, x+a, y+b)) {
                  rblxstbcc_update_grid(g, x+a, y+b, 0);
                  ++count;
               }
      } else {
         for (a=0; a < 1; ++a)
            for (b=0; b < 32; ++b)
               if (!rblxstbcc_query_grid_open(g, x+a, y+b)) {
                  rblxstbcc_update_grid(g, x+a, y+b, 0);
                  ++count;
               }
      }

      //if (i % 100 == 0) write_map(g, w, h, rblxstb_sprintf("tests/output/rblxstbcc/close_random_%d.png", i+1));
   }
   end_timer();
   write_map(g, w, h, rblxstb_sprintf("tests/output/rblxstbcc/close_random_%d.png", i));
   printf("Added %d grid spaces\n", count);
   #endif


   #if 0  // for map_02.png
   start_timer("process");
   for (k=0; k < 20; ++k) {
      for (j=0; j < h; ++j) {
         int any=0;
         for (i=0; i < w; ++i) {
            if (map[j*w+i] > 10 && map[j*w+i] < 250) {
               //start_timer(rblxstb_sprintf("open %d,%d", i,j));
               rblxstbcc_update_grid(g, i, j, 0);
               test_connected(g);
               //end_timer();
               any = 1;
            }
         }
         if (any) write_map(g, w, h, rblxstb_sprintf("tests/output/rblxstbcc/open_row_%04d.png", j));
      }

      for (j=0; j < h; ++j) {
         int any=0;
         for (i=0; i < w; ++i) {
            if (map[j*w+i] > 10 && map[j*w+i] < 250) {
               //start_timer(rblxstb_sprintf("close %d,%d", i,j));
               rblxstbcc_update_grid(g, i, j, 1);
               test_connected(g);
               //end_timer();
               any = 1;
            }
         }
         if (any) write_map(g, w, h, rblxstb_sprintf("tests/output/rblxstbcc/close_row_%04d.png", j));
      }
   }
   end_timer();
   #endif

   return 0;
}
#endif
