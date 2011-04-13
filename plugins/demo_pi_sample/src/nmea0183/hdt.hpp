#if ! defined( HDT_CLASS_HEADER )
#define HDT_CLASS_HEADER

/*
** Author: Samuel R. Blackburn
** CI$: 76300,326
** Internet: sammy@sed.csc.com
**
** You can use it any way you like.
*/

class HDT : public RESPONSE
{

   public:

      HDT();
     ~HDT();

      /*
      ** Data
      */

      double DegreesTrue;

      /*
      ** Methods
      */

      virtual void Empty( void );
      virtual bool Parse( const SENTENCE& sentence );
      virtual bool Write( SENTENCE& sentence );

      /*
      ** Operators
      */

      virtual const HDT& operator = ( const HDT& source );
};

#endif // HDT_CLASS_HEADER
