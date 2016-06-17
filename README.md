# trycatch.h

## #define TRYCATCH_NESTING Some_value
    
    //c code
    try {
    /* up to TRYCATCH_NESTING levels */ 
        try {
            throw( NotImplementedException );
        }
    /* see trycatch.conf for all valid values */
        catch( NotImplementedException ) {
            printf( "NotImplementedException\n" );
        }
        catch( Exception ) {
            printf( "General Exception " );
            if( __ex_with_msg ) printf( "with message: %s\n", __ex_msg );
            else printf( "\n" );
        }
        else {
            printf( "No Exception\n" );
        }
        finally {
            printf( "Finally block, REQUIRED IF TRYCATCH_NESTING IS SET." );
        }
        static char message[100];
        sprintf( message, "message with some data, %d, \"%s\"", 1, "@" );
        throw( Exception, message );
    } 
    catch( Exception ) {
        printf( "General Exception " );
        if( __ex_with_msg ) printf( "with message: %s\n", __ex_msg );
        else printf( "\n" );
    }
    finally;

## #undef TRYCATCH_NESTING

    //c code
    try {
        throw( NotImplementedException );
    }
    catch( NotImplementedException ) {
        printf( "NotImplementedException\n" );
    }
    catch( Exception ) {
        printf( "General Exception " );
        if( __ex_with_msg ) printf( "with message: %s\n", __ex_msg );
        else printf( "\n" );
    }
    else {
        printf( "No Exception\n" );
    }
    finally {
        printf( "Finally block, NOT required if TRYCATCH_NESTING is not set." );
    }
    