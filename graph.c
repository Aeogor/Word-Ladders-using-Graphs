/*graph.c*/

#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <limits.h>

#include "avl.h"
#include "stack.h"
#include "set.h"
#include "graph.h"
#include "mymem.h"



// #####################################################
//
// Graph:
//

//
// CreateGraph:
//
// Creates a directed graph with an initial capacity for N vertices;
// at this point graph is limited to at most N vertices.  The implementation
// is based on adjacency lists.
//
Graph *CreateGraph(int N)
{
  Graph *G;
  int    i;

  if (N < 1)
  {
    printf("\n**Error in CreateGraph: invalid parameter N (%d)\n\n", N);
    return NULL;
  }

  //
  // allocate graph header:
  //
  G = (Graph *)mymalloc(sizeof(Graph));
  if (G == NULL)
  {
    printf("\n**Error in CreateGraph: malloc failed to allocate\n\n");
    exit(-1);
  }


  //allocate array of adjacency lists, one per vertex:

  G->Vertices = (Edge **)mymalloc(N * sizeof(Edge *));
  if (G->Vertices == NULL)
  {
    printf("\n**Error in CreateGraph: malloc failed to allocate\n\n");
    exit(-1);
  }

  for (i = 0; i < N; ++i)  // initialize to empty lists:
    G->Vertices[i] = NULL;

  //
  // allocate array for storing vertex names:
  //
  G->Names = (char **)mymalloc(N * sizeof(char *));
  if (G->Names == NULL)
  {
    printf("\n**Error in CreateGraph: malloc failed to allocate\n\n");
    exit(-1);
  }

  for (i = 0; i < N; ++i)  // initialize to empty names:
    G->Names[i] = NULL;

  //
  // graph is empty to start --- initialize remaining fields:
  //
  G->NumVertices = 0;
  G->NumEdges = 0;
  G->Capacity = N;
  G->NamesTree = NULL;


  //done!

  return G;
}

//
// DeleteGraph:
//
// Frees the memory associated with this graph.
//
void DeleteGraph(Graph *G)
{
  int  i;

  //
  // Every vertex has a name, and a list of edges.  Free
  // that memory:
  //
  for (i = 0; i < G->NumVertices; ++i)
  {
    // free vertex name:
    myfree(G->Names[i]);

    // free each edge:
    Edge *cur, *temp;
    cur = G->Vertices[i];
    while (cur != NULL)
    {
      temp = cur;
      cur = cur->next;

      myfree(temp);
    }
  }
  FreeAVLTree(G->NamesTree);

  // free the arrays we just traversed:
  myfree(G->Vertices);
  myfree(G->Names);

  // free head node:
  myfree(G);

   //post-order like FatalError hinted at
}



//
// AddVertex:
//
// Adds a vertex with the given name to G, returning a unique integer id
// identifying this vertex.  Returns -1 if adding the vertex failed, i.e.
// if the graph is full and additional vertices cannot be created; this
// occurs when the graph's capacity is reached.
//
int AddVertex(Graph *G, char *name)
{
  int v = G->NumVertices;  // next free location:

  AVLElementType value;
  strcpy( value.Word, name);
  value.Vertex = v;

  G->NamesTree = Insert(G->NamesTree, value);

  if (G->NumVertices == G->Capacity)  // graph is full:
  {
    // we need to dynamically grow, so let's double in size:
    int N = 2 * G->Capacity;

    //
    // first we'll grow the array of names:
    //
    char **newNames = (char **)mymalloc(N * sizeof(char *));
    if (newNames == NULL)
    {
      printf("\n**Error in AddVertex: malloc failed to allocate\n\n");
      exit(-1);
    }

    // copy existing names over:
    int  i;

    for (i = 0; i < G->NumVertices; ++i)
    {
      newNames[i] = G->Names[i];
    }

    myfree(G->Names);

    //
    // now we need to grow the edge lists:
    //
    Edge **newVertices = (Edge **)mymalloc(N * sizeof(Edge *));
    if (newVertices == NULL)
    {
      printf("\n**Error in AddVertex: malloc failed to allocate\n\n");
      exit(-1);
    }

    // copy existing edge lists:
    for (i = 0; i < G->NumVertices; ++i)
    {
      newVertices[i] = G->Vertices[i];
    }

    myfree(G->Vertices);

    //
    // done, update graph header:
    //
    G->Names = newNames;
    G->Vertices = newVertices;
    G->Capacity = N;
  }

  // initialize edge list to empty:
  G->Vertices[v] = NULL;

  // make a copy of the name:
  G->Names[v] = (char *)mymalloc(((int)(strlen(name) + 1)) * sizeof(char));
  if (G->Names[v] == NULL)
  {
    printf("\n**Error in AddVertex: malloc failed to allocate\n\n");
    exit(-1);
  }

  strcpy(G->Names[v], name);

  // one more vertex now:
  G->NumVertices++;

  // done!  Return vertex's number:
  return v;
}

//
// Name2Vertex:
//
// Looks up a vertex by name, returning its vertex #
// if found -- this value will be >= 0.  Returns -1 if
// not found.
//
int Name2Vertex(Graph *G, char *Name)
{
  // int  i;
  //
  // //
  // // linear search through the Names array:
  // //
  // for (i = 0; i < G->NumVertices; ++i)
  // {
  //   if (strcmp(G->Names[i], Name) == 0)
  //     return i;
  // }

  AVLNode *cur = G->NamesTree;

  while (cur != NULL)
  {
    if (strcmp(Name, cur->value.Word) == 0)  // match!
      return cur->value.Vertex;
    else if (strcmp(Name, cur->value.Word) < 0)  // smaller, go left:
      cur = cur->left;
    else  // larger, go right:
      cur = cur->right;
  }


  // if get here, not found:
  return -1;
}

//
// Vertex2Name:
//
// Looks up a vertex by number, returning a pointer to
// its name; returns NULL if v is invalid.
//
// NOTE: do not change the string via the returned
// pointer; treat the pointer and the underlying string
// read-only.
//
char *Vertex2Name(Graph *G, Vertex v)
{
  if (v < 0 || v >= G->NumVertices)
    return NULL;

  return G->Names[v];
}

//
// AddEdge:
//
// Adds a directed edge (src, dest, weight) to G.  If successful, true
// (non-zero) is returned; if the edge could not be added (i.e. due to
// invalid vertex ids), then false (0) is returned.
//
// NOTE: loops and multi-edges are allowed.  To allow easier handling of
// multi-edges, edges are stored "in order by destination".  So when
// inserting a new edge E = (s,d,w), insert this edge in the list such
// that E's position in the list is ordered by "d".  Example: if the list
// contains (0,1,100)->(0,2,75), and the new edge is (0,2,150), then the
// resulting list is (0,1,100)->(0,2,150)->(0,2,75).  The new edge is
// inserted *before* existing edges with same or larger destination.
//
int AddEdge(Graph *G, Vertex src, Vertex dest, int weight)
{
  if (src < 0 || src >= G->NumVertices)  // invalid vertex #:
    return 0;
  if (dest < 0 || dest >= G->NumVertices)  // invalid vertex #:
    return 0;

  //
  // allocate memory for new edge:
  //
  Edge *edge = (Edge *)mymalloc(sizeof(Edge));
  if (edge == NULL)
  {
    printf("\n**Error in AddEdge: malloc failed to allocate\n\n");
    exit(-1);
  }

  //
  // store data:
  //
  edge->src = src;
  edge->dest = dest;
  edge->weight = weight;

  //
  // link into edge list --- we want to insert in order so that we
  // can detect multi-edges more easily (i.e. they will be consecutive
  // in the adjacency list):
  //
  Edge *prev = NULL;
  Edge *cur = G->Vertices[src];

  while (cur != NULL)
  {
    if (dest <= cur->dest)  // insert here!
      break;

    // else keep going:
    prev = cur;
    cur = cur->next;
  }

  if (prev == NULL)  // update head pointer:
  {
    edge->next = G->Vertices[src];
    G->Vertices[src] = edge;
  }
  else  // insert between prev and cur:
  {
    edge->next = prev->next;
    prev->next = edge;
  }

  //
  // done:
  //
  G->NumEdges++;

  return 1;  // success!
}

//
// Neighbors:
//
// Returns the neighbors of a given vertex --- i.e. the vertices
// adjacent to v.  The neighbors are returned in a dynamically-
// allocated array, in ascending order; the vertices are followed
// by -1 to denote the end of the data.  A vertex appears at most
// once in the returned array, even in the presence of multi-edges.
//
// NOTE: returns NULL if v is not a valid vertex id.
//
// NOTE: it is the responsibility of the CALLER to free the returned
// array when they are done.
//
Vertex *Neighbors(Graph *G, Vertex v)
{
  Vertex *neighbors;
  int     N;
  int     i;

  if (v < 0 || v >= G->NumVertices)  // invalid vertex #:
    return NULL;

  //
  // allocate array of worst-case size: # of vertices + 1
  //
  N = G->NumVertices + 1;

  neighbors = (Vertex *)mymalloc(N * sizeof(Vertex));
  if (neighbors == NULL)
  {
    printf("\n**Error in Neighbors: malloc failed to allocate\n\n");
    exit(-1);
  }

  //
  // Now loop through the list of edges and copy the dest
  // vertex of each edge:
  //
  Edge *cur = G->Vertices[v];

  i = 0;
  while (cur != NULL)  // for each edge out of v:
  {
    //
    // the dest is our neighbor --- however, we have to be
    // careful of multi-edges, i.e. edges with the same dest.
    // Since edges are stored in order, edges with same dest
    // appear next to each other in the list --- so look to
    // see if array already contains dest before we copy over:
    //
    if (i == 0)  // first neighbor, we always copy:
    {
      neighbors[i] = cur->dest;
      ++i;
    }
    else if (neighbors[i - 1] != cur->dest)  // make sure not a multi-edge:
    {
      neighbors[i] = cur->dest;
      ++i;
    }
    else // multi-edge, so ignore:
      ;

    cur = cur->next;
  }

  //
  // follow last element with -1 and return:
  //
  neighbors[i] = -1;

  return neighbors;
}

///
// Prints the graph for debugging purposes.  Pass true
// (non-zero) for the "complete" parameter to dump complete
// info --- e.g. BFS and DFS from each vertex --- otherwise
// pass false (0) for just graph stats.
//
void PrintGraph(Graph *G, char *title, int complete)
{
  printf(">>Graph: %s\n", title);
  printf("  # of vertices: %d\n", G->NumVertices);
  printf("  # of edges:    %d\n", G->NumEdges);

  // is a complete print desired?  if not, return now:
  if (!complete)
    return;

  //
  // Otherwise dump complete graph info:
  //
  printf("  Adjacency Lists:\n");

  int  v;
  for (v = 0; v < G->NumVertices; ++v)
  {
    printf("   %d (%s): ", v, G->Names[v]);

    Edge *edge = G->Vertices[v];
    while (edge != NULL)
    {
      printf("(%d,%d,%d)", edge->src, edge->dest, edge->weight);

      edge = edge->next;
      if (edge != NULL)
        printf(", ");
    }

    printf("\n");
  }

  //
  // neighbors:
  //
  printf("  Neighbors:\n");

  for (v = 0; v < G->NumVertices; ++v)
  {
    printf("   %d (%s): ", v, G->Names[v]);

    Vertex *neighbors = Neighbors(G, v);

    if (neighbors == NULL)
      printf("**ERROR: Neighbors returned NULL.\n\n");
    else
    {
      int  j;

      for (j = 0; neighbors[j] != -1; ++j)
      {
        printf("%d, ", neighbors[j]);
      }

      printf("-1\n");

      myfree(neighbors);
    }
  }

  //
  // BFS:
  //
  printf("  BFS:\n");

  for (v = 0; v < G->NumVertices; ++v)
  {
    printf("   %d (%s): ", v, G->Names[v]);

    Vertex *visited = BFS(G, v);

    if (visited == NULL)
      printf("**ERROR: BFS returned NULL.\n\n");
    else
    {
      int  j;

      for (j = 0; visited[j] != -1; ++j)
      {
        printf("%d, ", visited[j]);
      }

      printf("-1\n");

      myfree(visited);
    }
  }

  //
  // DFS:
  //
  printf("  DFS:\n");

  for (v = 0; v < G->NumVertices; ++v)
  {
    printf("   %d (%s): ", v, G->Names[v]);

    Vertex *visited = DFS(G, v);

    if (visited == NULL)
      printf("**ERROR: DFS returned NULL.\n\n");
    else
    {
      int  j;

      for (j = 0; visited[j] != -1; ++j)
      {
        printf("%d, ", visited[j]);
      }

      printf("-1\n");

      myfree(visited);
    }
  }

}

//
// BFS:
//
// Performs a breadth-first search starting from vertex v,
// returning a dynamically-allocated array of the vertices
// visited, in the order they are visited.  Note that v will
// appear first in the returned array, and the last vertex
// is followed by -1 to denote the end.  When the neighbors
// of a vertex are visited, they are done so in ascending
// order; no vertex is visited more than once, even in the
// presence of cycles and multi-edges.
//
// NOTE: returns NULL if v is not a valid vertex id.
//
// NOTE: it is the responsibility of the CALLER to free the
// returned array when they are done.
//
Vertex *BFS(Graph *G, Vertex v)
{
  Vertex *visited;
  int     N;
  int     i;

  if (v < 0 || v >= G->NumVertices)  // invalid vertex #:
    return NULL;

  //
  // allocate array of worst-case size: # of vertices + 1
  //
  N = G->NumVertices + 1;

  visited = (Vertex *)mymalloc(N * sizeof(Vertex));
  if (visited == NULL)
  {
    printf("\n**Error in BFS: malloc failed to allocate\n\n");
    exit(-1);
  }

  //
  // Perform BFS, starting at given vertex v:
  //
  Queue *frontierQ = CreateQueue(N);
  Set   *discoveredSet = CreateSet(N);

  if (!Enqueue(frontierQ, v)) { printf("Error!\n"); exit(-1); }
  if (!AddToSet(discoveredSet, v)) { printf("Error!\n"); exit(-1); }

  i = 0;  // index into visited of where next vertex goes:

  while (!isEmptyQueue(frontierQ))
  {
    Vertex currentV = Dequeue(frontierQ);

    // visit:  add to visited list:
    visited[i] = currentV;
    ++i;

    Vertex *neighbors = Neighbors(G, currentV);

    int j = 0;  // index into array of neighbors:
    while (neighbors[j] != -1)
    {
      Vertex adjV = neighbors[j];

      if (!isElementInSet(discoveredSet, adjV))
      {
        if (!Enqueue(frontierQ, adjV)) { printf("Error!\n"); exit(-1); }
        if (!AddToSet(discoveredSet, adjV)) { printf("Error!\n"); exit(-1); }
      }

      ++j;
    }

    myfree(neighbors);
  }//while

   //
   // done:
   //
  visited[i] = -1;  // mark end of vertices with -1:

  DeleteQueue(frontierQ);
  DeleteSet(discoveredSet);

  return visited;
}


//
// BFSd:
//
// Performs a breadth-first search starting from vertex v,
// returning a dynamically-allocated array of the vertices
// visited, in the order they are visited.  Unlike BFS(),
// this function stops after walking "d" steps away from v.
// A "step" is an edge, which implies that d=2 => visiting
// all nodes 2 edges away from v.
//
// The returned array contains "markers" to denote vertices
// that are 0 steps away, 1 step away, 2 steps away, etc.
// Each marker is -1, so given a distance d, the caller should
// loop through the returned vertex until d+1 markers are
// processed.  Then stop.  Example: d=2 => 3 markers,
// after step 0, step 1, and step 2.
//
// NOTE: returns NULL if v is not a valid vertex id, or
// if distance < 1.
//
// NOTE: it is the responsibility of the CALLER to free the
// returned array when they are done.
//
Vertex *BFSd(Graph *G, Vertex v, int distance)
{
  Vertex *visited;
  int     N;
  int     i;

  if (v < 0 || v >= G->NumVertices)  // invalid vertex #:
    return NULL;

  if (distance < 1)
    return NULL;

  //
  // allocate array of worst-case size: # of vertices + distance + 1
  //
  N = G->NumVertices + distance + 1;

  visited = (Vertex *)mymalloc(N * sizeof(Vertex));
  if (visited == NULL)
  {
    printf("\n**Error in BFS: malloc failed to allocate\n\n");
    exit(-1);
  }

  //
  // Perform BFS, starting at given vertex v:
  //
  Queue *frontierQ = CreateQueue(N);
  Set   *discoveredSet = CreateSet(N);

  if (!Enqueue(frontierQ, v)) { printf("Error!\n"); exit(-1); }
  if (!AddToSet(discoveredSet, v)) { printf("Error!\n"); exit(-1); }

  i = 0;  // index into visited of where next vertex goes:

  Enqueue(frontierQ, -1);

  while (!isEmptyQueue(frontierQ))
  {

    if (frontierQ->Elements[frontierQ->Front] == -1)
    {
      visited[i] = -1;
      ++i;

      distance--;
      if (distance < 0)
        break;

      Dequeue(frontierQ);
      Enqueue(frontierQ, -1);

      continue;  // skip loop body and repeat:
    }

    Vertex currentV = Dequeue(frontierQ);

    // visit:  add to visited list:
    visited[i] = currentV;
    ++i;

    Vertex *neighbors = Neighbors(G, currentV);

    int j = 0;  // index into array of neighbors:
    while (neighbors[j] != -1)
    {
      Vertex adjV = neighbors[j];

      if (!isElementInSet(discoveredSet, adjV))
      {
        if (!Enqueue(frontierQ, adjV)) { printf("Error!\n"); exit(-1); }
        if (!AddToSet(discoveredSet, adjV)) { printf("Error!\n"); exit(-1); }
      }

      ++j;
    }

    myfree(neighbors);

  }//while

   //
   // done:
   //
  visited[i] = -1;  // mark end of vertices with -1:

  DeleteQueue(frontierQ);
  DeleteSet(discoveredSet);

  return visited;
}

//
// DFS:
//
// Performs a depth-first search starting from vertex v,
// returning a dynamically-allocated array of the vertices
// visited, in the order they are visited.  Note that v will
// appear first in the returned array, and the last vertex
// is followed by -1 to denote the end; a vertex appears
// at most once in the visited array.  When the neighbors
// of a vertex are visited, they are done so in ascending
// order.
//
// NOTE: returns NULL if v is not a valid vertex id.
//
// NOTE: it is the responsibility of the CALLER to free the
// returned array when they are done.
//
Vertex *DFS(Graph *G, Vertex v)
{
  if (v < 0 || v >= G->NumVertices)  // invalid vertex #:
    return NULL;

  //
  // Perform DFS, starting at given vertex v:
  //
  int N = G->NumVertices + 1;

  Stack *frontierStack = CreateStack(N);
  Set   *visitedSet = CreateSet(N);
  Queue *visitedQ = CreateQueue(N);

  if (!Push(frontierStack, v)) { printf("Error!\n"); exit(-1); }

  while (!isEmptyStack(frontierStack))
  {
    Vertex currentV = Pop(frontierStack);

    //
    // visit:  add to visited list *if* not already there, since
    // DFS may end up visiting vertices multiple times:
    //
    if (!isElementInQueue(visitedQ, currentV))
    {
      if (!Enqueue(visitedQ, currentV)) { printf("Error!\n"); exit(-1); }
    }

    //
    // now push neighbors in reverse order so that we visit in
    // ascending order:
    //
    if (!isElementInSet(visitedSet, currentV))
    {
      if (!AddToSet(visitedSet, currentV)) { printf("Error!\n"); exit(-1); }

      Vertex *neighbors = Neighbors(G, currentV);

      //
      // Note: push them backwards onto stack so vertices are
      // on the stack in ascending order:
      //
      int j = 0;  // index into array of neighbors:

      while (neighbors[j] != -1)  // find the end
        j++;

      j--;  // advance to last vertex in the visited array:

      while (j >= 0)  // now push backwards:
      {
        Vertex adjV = neighbors[j];

        if (!Push(frontierStack, adjV)) { printf("Error!\n"); exit(-1); }

        --j;
      }

      myfree(neighbors);
    }
  }//while

  //
  // done: our set of visited vertices is in visitedQ, so let's
  // copy out of there into a dynamically-allocated array:
  //

  //
  // allocate array of worst-case size: # of vertices + 1
  //
  Vertex *visited;

  visited = (Vertex *)mymalloc(N * sizeof(Vertex));
  if (visited == NULL)
  {
    printf("\n**Error in DFS: malloc failed to allocate\n\n");
    exit(-1);
  }

  // dequeue vertices into the visited array:
  int i = 0;  // index into visited array:

  while (!isEmptyQueue(visitedQ))
  {
    visited[i] = Dequeue(visitedQ);
    ++i;
  }

  visited[i] = -1;  // mark end of vertices with -1:

  //
  // Done:
  //
  DeleteStack(frontierStack);
  DeleteSet(visitedSet);
  DeleteQueue(visitedQ);

  return visited;
}


//------------------------------------Dijkstra ALGORITHM---------------------------------------------------//
//
// getEdgeWeight:
//
// Returns the weight along edge src -> dest.  If there are
// multiple edges, returns the minimum weight; if there is no
// such edge, an error message is printed and the program is
// exited.
//
// NOTE: the error message and program exit occur since you
// should never call function unless the edge exists.
//
int getEdgeWeight(Graph *G, Vertex src, Vertex dest)
{
  if (src < 0 || src >= G->NumVertices)  // invalid vertex #:
  {
    printf("\n**Error in getEdgeWeight: src vertex (%d) invalid.\n\n", src);
    exit(-1);
  }
  if (dest < 0 || dest >= G->NumVertices)  // invalid vertex #:
  {
    printf("\n**Error in getEdgeWeight: dest vertex (%d) invalid.\n\n", dest);
    exit(-1);
  }

  //
  // search src's edge list, note that multi-edges appear together:
  //
  Edge *cur = G->Vertices[src];
  int   weight;
  int   haveEdge = 0;  /*false*/

  while (cur != NULL)
  {
    if (dest == cur->dest)  // candidate edge:
    {
      if (!haveEdge) // first edge:
      {
        haveEdge = 1;  /*true*/
        weight = cur->weight;
      }
      else if (cur->weight < weight)  // multi-edge:
      {
        weight = cur->weight;  // smaller, so update:
      }
    }
    else if (dest < cur->dest)  // out of order, end search:
      break;

    // next edge:
    cur = cur->next;
  }

  //
  // did we find an edge?  make sure...
  //
  if (!haveEdge)
  {
    printf("\n**Error in getEdgeWeight: no edge found from %d to %d.\n\n", src, dest);
    exit(-1);
  }

  //
  // success:
  //
  return weight;
}


//
// Performs Dijkstra's shortest path algorithm to find the shortest path
// from src to dest.  Returns a dynamically-allocated array of vertices
// denoting this path; the array will start with src, contain 0 or more
// vertices that lead to dest, followed by dest, and ending with -1.
// If there is no pat from src to dest, the array will contain only -1.
//
// NOTE: returns NULL if src or dest are not valid vertex ids.
//
// NOTE: it is the responsibility of the CALLER to free the
// returned array when they are done.
//
int PopMin(Queue *unvisitedQ, int distance[])
{
  int    N = unvisitedQ->NumElements;
  Stack *S = CreateStack(N);

  assert(!isEmptyQueue(unvisitedQ));

  //
  // search for smallest vertex distance in the queue,
  // saving other elements in a stack so we can put back
  // later:
  //
  int minV = Dequeue(unvisitedQ);  // assume first vertex is the min:

  while (!isEmptyQueue(unvisitedQ))  // look for smaller vertex:
  {
    int v = Dequeue(unvisitedQ);

    if (distance[v] < distance[minV])
    {
      Push(S, minV);  // save so we can put back in queue:
      minV = v;  // our new min:
    }
    else  // not smaller, so save to enqueue later:
    {
      Push(S, v);
    }
  }

  //
  // empty the stack back into the queue:
  //
  while (!isEmptyStack(S))
  {
    Enqueue(unvisitedQ, Pop(S));
  }

  // NOTE: at this point queue is now one smaller, with
  // min element removed.  Let's confirm:
  assert(unvisitedQ->NumElements == (N - 1));

  //
  // done:
  //
  DeleteStack(S);

  return minV;
}


Vertex *Dijkstra(Graph *G, Vertex src, Vertex dest)
{
  int  INF = INT_MAX;

  if (src < 0 || src >= G->NumVertices)  // invalid vertex #:
    return NULL;
  if (dest < 0 || dest >= G->NumVertices)  // invalid vertex #:
    return NULL;

  //
  // allocate distances array:
  //
  int N = G->NumVertices;

  int *distance = (int *)mymalloc(N * sizeof(int));
  if (distance == NULL)
  {
    printf("\n**Error in Dijkstra: malloc failed to allocate\n\n");
    exit(-1);
  }

  Vertex *predecessor = (Vertex *)mymalloc(N * sizeof(Vertex));
  if (predecessor == NULL)
  {
    printf("\n**Error in Dijkstra: malloc failed to allocate\n\n");
    exit(-1);
  }

  //
  // for each vertex, add to Queue to be visited,
  // initialize distance to Infinity, and set predecessor
  // to -1:
  //
  Queue *unvisitedQ = CreateQueue(N);
  int currentV;

  for (currentV = 0; currentV < N; ++currentV)
  {
    Enqueue(unvisitedQ, currentV);
    distance[currentV] = INF;
    predecessor[currentV] = -1;
  }

  //
  // starting vertex has a distance of 0 from itself:
  //
  distance[src] = 0;

  //
  // Now run Dijkstra's algorithm:
  //
  while (!isEmptyQueue(unvisitedQ))
  {
    //
    // find the vertex with the smallest distance from
    // the start, that's the vertex to explore next:
    //
    currentV = PopMin(unvisitedQ, distance);

    // is there a path to the current vertex?  if not,
    // there's no point in looking at neighbors...
    if (distance[currentV] == INF)
      break;

    //
    // now see if we have found any shorter paths for minV's
    // neighboring vertices:
    //
    Vertex *neighbors = Neighbors(G, currentV);

    int i = 0;
    while (neighbors[i] != -1)  // for each neighbor:
    {
      int adjV = neighbors[i];

      int edgeWeight = getEdgeWeight(G, currentV, adjV);
      int altDistance = distance[currentV] + edgeWeight;

      if (altDistance < distance[adjV])
      {
        distance[adjV] = altDistance;
        predecessor[adjV] = currentV;
      }

      ++i;
    }

    myfree(neighbors);
  }

  //
  // Okay, algorithm has run to completion, and the path (if
  // any) is stored backwards in predecessor array.  So let's
  // use a stack to reverse it:
  //
  Stack *S = CreateStack(N);

  int v = dest;
  while (predecessor[v] != -1)
  {
    Push(S, v);
    v = predecessor[v];
  }

  // loop stops when it gets to src, so push src to finish:
  Push(S, src);

  //
  // at this point we have the path on the stack, or if there's
  // no path, just src is on the stack.  Allocated an array and
  // fill with path, or if no path, just -1:
  //
  Vertex  *path;

  if (S->NumElements == 1) // just src on stack, no path:
  {
    N = 1;  // just the -1:

    path = (Vertex *)mymalloc(N * sizeof(Vertex));
    if (path == NULL)
    {
      printf("\n**Error in Dijkstra: malloc failed to allocate\n\n");
      exit(-1);
    }

    path[0] = -1;  // no path from src to dest:
  }
  else
  {
    N = S->NumElements + 1;  // path + -1 at the end

    path = (Vertex *)mymalloc(N * sizeof(Vertex));
    if (path == NULL)
    {
      printf("\n**Error in Dijkstra: malloc failed to allocate\n\n");
      exit(-1);
    }

    // now empty the stack into the path array:
    int i = 0;
    while (!isEmptyStack(S))
    {
      path[i] = Pop(S);
      ++i;
    }

    path[i] = -1;  // need -1 terminator at the end:
  }

  //
  // done!
  //
  DeleteStack(S);
  DeleteQueue(unvisitedQ);
  myfree(distance);
  myfree(predecessor);

  return path;
}
