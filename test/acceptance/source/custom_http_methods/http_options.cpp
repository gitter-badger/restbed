/*
 * Copyright (c) 2013, 2014, 2015 Corvusoft
 */

//System Includes
#include <thread>
#include <string>
#include <memory>
#include <stdexcept>
#include <functional>

//Project Includes
#include <restbed>

//External Includes
#include <catch.hpp>
#include <corvusoft/framework/http>

//System Namespaces
using std::thread;
using std::string;
using std::shared_ptr;
using std::make_shared;

//Project Namespaces
using namespace restbed;

//External Namespaces
using namespace framework;

void invoke_handler( const shared_ptr< Session >& session )
{
    session->close( 200, "Hello, World!", { { "Content-Length", "13" } } );
}

SCENARIO( "publishing custom HTTP methods", "[resource]" )
{
    GIVEN( "I publish a resource at '/resources/1' with a HTTP 'INVOKE' method handler" )
    {
        auto resource = make_shared< Resource >( );
        resource->set_path( "/resources/1" );
        resource->set_method_handler( "INVOKE", invoke_handler );

        auto settings = make_shared< Settings >( );
        settings->set_port( 1984 );
        settings->set_default_header( "Connection", "close" );

        Service service;
        service.publish( resource );

        thread service_thread( [ &service, settings ] ( )
        {
            service.start( settings );
        } );

        WHEN( "I perform a HTTP 'OPTIONS' request to '/resources/1'" )
        {
            Http::Request request;
            request.port = 1984;
            request.host = "localhost";
            request.path = "/resources/1";

            auto response = Http::options( request );

            THEN( "I should see a '501' (Not Implemented) status code" )
            {
                REQUIRE( 501 == response.status_code );
            }

            AND_THEN( "I should see an empty repsonse body" )
            {
                REQUIRE( response.body.empty( ) );
            }

            AND_THEN( "I should see a 'Connection' header value of 'close'" )
            {
                auto header = response.headers.find( "Connection" );
                REQUIRE( header not_eq response.headers.end( ) );
                REQUIRE( "close" == response.headers.find( "Connection" )->second );
            }

            AND_THEN( "I should not see a 'Content-Length' header value" )
            {
                REQUIRE( response.headers.find( "Content-Length" ) == response.headers.end( ) );
            }
        }

        service.stop( );
        service_thread.join( );
    }
}
