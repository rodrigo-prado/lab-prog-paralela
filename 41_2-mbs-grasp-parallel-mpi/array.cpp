//---------------------------------------------------------------------------

#ifndef ARRAY
#define ARRAY

//---------------------------------------------------------------------------

#include <iostream>
#include <new>

//---------------------------------------------------------------------------

extern void mserro(const char *clas,const char *func,const char * ms,const int nerr);

//---------------------------------------------------------------------------

#ifdef DEBUG_ALL
#define DEBUG_ARRAY

extern int nfree;
extern int naloc;
#endif

//---------------------------------------------------------------------------

template <class T>
class Array
{
public:
  Array():pType(NULL),size(0){}
  Array(const int tam, T val = 0);
  Array(const Array &rhs);
  ~Array() {desaloca();}
  Array& operator= (const Array&);
  T& operator[](int i);
  const T& operator[](int i) const;
  int getsize() const {return size;}
  T* operator&() const {return pType;}
  int gera_key1(const int tam=0);  // retorna a soma de todos os elementos
  void realoca(const int novo_tam, const T val = 0); // realoca o vetor copiando os valores anteriores
  void redefine(const int novo_tam, const T val = 0); // aloca o vetor e os elementos ficam igual a val
  void aloca(const int novo_tam);  // aloca o vetor
  void ajusta(const int novo_tam); // mesma coisa que realoca mas diminui o tamanho
  void set(const T val = 0);  // todo o vetor fica igual a val
  void set(const T val, const int fim, const int ini); // o vetor fica igual a val entre ini e fim
  void desaloca() {del(); pType = NULL; size = 0;}
  void imprime();
  Array* get_t()  { Array<T> *p; p = this; return p;}
private:
  void del();
  T *pType;
  int size;
};

//---------------------------------------------------------------------------

template <class T>
Array<T>::Array(const int tam, const T val):
  pType(NULL),size(0)
{
  redefine(tam, val);
}

//---------------------------------------------------------------------------

template <class T>
Array<T>::Array(const Array &rhs)
{
#ifdef DEBUG_ARRAY
  std::cout<<"Array copy constructor !!!!!!!!!!!!!"<<std::endl;
#endif
  pType = NULL;
  aloca(rhs.getsize());
  for(int i=0;i<size;++i)
    pType[i] = rhs[i];
}

//---------------------------------------------------------------------------

template <class T>
T& Array<T>::operator[](int i)
{
#ifdef DEBUG_ARRAY
  if(i >= size || i < 0)
    mserro("Array","[]","[i] invalido",5);
#endif
 return pType[i];
}

//---------------------------------------------------------------------------

template <class T>
const T& Array<T>::operator[](int i) const
{
#ifdef DEBUG_ARRAY
  if(i >= size || i< 0)
    mserro("Array","[] const","[i] invalido",5);
#endif
 return pType[i];
}

//---------------------------------------------------------------------------

template <class T>
Array<T>& Array<T>::operator=(const Array &rhs)
{     
#ifdef DEBUG_ARRAY
  std::cout<<"Array operator= !!!!!!!!!!!!!"<<std::endl;
#endif
  //if(this == &(rhs))
  //return *this;
  //del();
  //pType = NULL;
  aloca(rhs.getsize());
  for(int i=0;i<size;++i)
    pType[i] = rhs[i];
  return *this;
}

//---------------------------------------------------------------------------

template <class T>
int Array<T>::gera_key1(const int tam)
{
  int t,key1 = 0;

  if(tam != 0)
    t = tam;
  else
    t = size;
  for(int i=0;i<t;++i)
    key1 += pType[i];
  return key1;
}

//---------------------------------------------------------------------------

template <class T>
void Array<T>::realoca(const int novo_tam, const T val)
{
  int i;

  if(novo_tam <= size)
    mserro("Array","realoca","Tamalho menor do que o existente",9);
  T* novo_pType = new(std::nothrow) T[novo_tam];
  if(novo_pType == NULL)
    mserro("Array","realoca","Falta de memoria",0);
#ifdef DEBUG_ARRAY
  ++naloc;
#endif

  for(i=0;i<size;++i)
    novo_pType[i] = pType[i];
  for(i=size;i<novo_tam;++i)
    novo_pType[i] = val;
  del();
  size = novo_tam;
  pType = novo_pType;
}

//---------------------------------------------------------------------------

template <class T>
void Array<T>::aloca(const int novo_tam)
{  
  del();
  if(novo_tam != 0)
    {
      pType = new(std::nothrow) T[novo_tam];
      if(pType == NULL)
	{
	  std::cout<<"tam = "<<novo_tam<<std::endl;
	mserro("Array","aloca","Falta de memoria",0);
	}
#ifdef DEBUG_ARRAY
      ++naloc;
#endif
    }
  else
    pType = NULL;
  size = novo_tam;
}

//---------------------------------------------------------------------------

template <class T>
void Array<T>::redefine(const int novo_tam, const T val)
{
  aloca(novo_tam);
  set(val);
}

//---------------------------------------------------------------------------

template <class T>
void Array<T>::set(const T val)
{
  for(int i=0;i<size;++i)
    pType[i] = val;
}

//---------------------------------------------------------------------------

template <class T>
void Array<T>::set(const T val, const int fim, const int ini)
{
#ifdef DEBUG_ARRAY
  if(ini < 0 || fim > size)
    mserro("Array","set","liminte invalido",9);
#endif
  for(int i=ini;i<fim;++i)
    pType[i] = val;
}

//---------------------------------------------------------------------------

template <class T>
void Array<T>::del()
{
  if(pType != NULL)
    {
      delete [] pType;
#ifdef DEBUG_ARRAY
      ++nfree;
#endif
    }
}

//---------------------------------------------------------------------------

template <class T>
void Array<T>::ajusta(const int novo_tam)
{
  T* novo_pType;

  if(novo_tam == size) return;

  if(novo_tam > size)
    mserro("Array","ajusta","Tamalho maior do que o existente",9);

  if(novo_tam != 0)
    {
      novo_pType = new(std::nothrow) T[novo_tam];
      if(novo_pType == NULL)
	mserro("Array","ajusta","Falta de memoria",0);
#ifdef DEBUG_ARRAY
      ++naloc;
#endif

      for(int i=0;i<novo_tam;++i)
	novo_pType[i] = pType[i];
    }
  else
    novo_pType = NULL;
  del();
  size = novo_tam;
  pType = novo_pType;
}

//---------------------------------------------------------------------------

template <class T>
void Array<T>::imprime()
{
  std::cout<<std::endl;
  for(int i=0;i<size;++i)
    std::cout<<i<<") "<<pType[i]<<", ";
  std::cout<<std::endl;
}

//---------------------------------------------------------------------------


#endif

//---------------------------------------------------------------------------
