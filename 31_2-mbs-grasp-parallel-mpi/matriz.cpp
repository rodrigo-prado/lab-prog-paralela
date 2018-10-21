//---------------------------------------------------------------------------

#ifndef MATRIZ
#define MATRIZ

//---------------------------------------------------------------------------

#include <new>
#include <iostream> 

//---------------------------------------------------------------------------

#ifdef DEBUG_ALL
#define DEBUG_MATRIZ

extern int naloc;
extern int nfree;
#endif

//---------------------------------------------------------------------------

extern void mserro(const char *clas,const char *func,const char * ms,const int nerr);

//---------------------------------------------------------------------------
template <class T>
class Matriz
{
public:
  Matriz():pType(NULL),n(0),m(0){}
  Matriz(const int nin, const int min, const T val = 0);
  ~Matriz();
  T& operator()(const int i, const int j);
  const T& operator()(const int i, const int j) const;
  void redefine(const int novo_n, const int novo_m, const T val = 0); // aloca a matriz e os elementos ficam igual a val
  void aloca(const int novo_n, const int novo_m);  // aloca a matriz
  void set(const T val = 0);  // toda a matriz fica igual a val
  void set(const int i, const T val);
  void get_size(int &nout, int &mout) {nout = n; mout = m;}
  void desaloca() {del(); n=m = 0; pType = NULL;}
  void imprime();
private:
  void del();
  T **pType;
  int n; // numero de linhas
  int m; // nomero de colunas
};

//---------------------------------------------------------------------------

template <class T>
Matriz<T>::Matriz(const int nin, const int min, const T val)
{
  pType = NULL; n = m = 0;
  redefine(nin, min, val);
}

//---------------------------------------------------------------------------

template<class T>
Matriz<T>::~Matriz()
{
  del();
  pType = NULL;
  n = m = 0;
}

//---------------------------------------------------------------------------

template <class T>
T& Matriz<T>::operator()(const int i, const int j)
{
#ifdef DEBUG_MATRIZ
  if(i >= n || j >= m || i < 0 || j < 0)
    mserro("Matriz", "operator()", "i e/ou j invalidos", 5);
#endif
 return pType[i][j];
}

//---------------------------------------------------------------------------

template <class T>
const T& Matriz<T>::operator()(const int i, const int j) const
{
#ifdef DEBUG_MATRIZ
  if(i >= n || j >= m || i < 0 || j < 0)
    mserro("Matriz", "operator() const", "i e/ou j invalidos", 5);
#endif
  return pType[i][j];
}

//---------------------------------------------------------------------------

template <class T>
void Matriz<T>::aloca(const int novo_n, const int novo_m)
{  
  del();
  if(novo_n + novo_m != 0)
    {
#ifdef DEBUG_MATRIZ
      if(novo_n <= 0 || novo_m <= 0)
	mserro("Matriz", "aloca", "n e/ou m invalidos", 9);
      ++naloc;
#endif
      pType = new T*[novo_n];
      if(pType == NULL)
	mserro("Matriz", "aloca", "Falta de memoria 1", 0);
      for(int i=0;i<novo_n;i++)
        {
          pType[i] = new T[novo_m];
          if(pType[i] == NULL)
	    mserro("Matriz", "aloca", "Falta de memoria 2", 0);
#ifdef DEBUG_MATRIZ
	  ++naloc;
#endif
        }
    }
  else
    pType = NULL;
  n = novo_n;
  m = novo_m;
}

//---------------------------------------------------------------------------

template <class T>
void Matriz<T>::redefine(const int novo_n, const int novo_m, const T val)
{
  aloca(novo_n, novo_m);
  set(val);
}

//---------------------------------------------------------------------------

template <class T>
void Matriz<T>::set(const T val)
{
  for(int i=0;i<n;++i)
    for(int j=0;j<m;++j)
      pType[i][j] = val;
}

//---------------------------------------------------------------------------

template <class T>
void Matriz<T>::set(const int i, const T val)
{
  for(int j=0;j<m;++j)
    pType[i][j] = val;
}

//---------------------------------------------------------------------------

template <class T>
void Matriz<T>::del()
{
  if(pType != NULL)
    {
      for(int i=0;i<n;++i)
	{
	  delete [] pType[i];
#ifdef DEBUG_MATRIZ
	  ++nfree;
#endif
	}
      delete [] pType;
#ifdef DEBUG_MATRIZ
      ++nfree;
#endif
    }
}

//---------------------------------------------------------------------------


template <class T>
void Matriz<T>::imprime()
{
  std::cout<<"--- MATRIZ ---"<<std::endl;
  for(int i=0;i<n;++i)
    {
      std::cout<<i<<") ";
      for(int j=0;j<m;++j)
	{
	  std::cout<<pType[i][j]<<", ";
	}
      std::cout<<std::endl;
    }
}


#endif

//---------------------------------------------------------------------------
