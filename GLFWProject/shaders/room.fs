#version 330 core

in vec2 pos;

out vec4 fragColour;

uniform sampler2D layoutTexture;
uniform sampler2D tilesetTexture;
uniform vec3 tileSetVals; // tileSize, numTilesInTilesetRow, textureSize
uniform vec2 mapSizeInTiles;

uniform vec3 cameraPos;
uniform vec2 viewport;

void main(){
    vec2 tileTex = texture( layoutTexture, pos ).xy;

    // White pixel indicates an empty tile.
    if( tileTex.x == 1.0 && tileTex.y == 1.0 )
    {
        discard;
    }

    vec2 tileID = vec2( floor( tileTex.x * tileSetVals.y ), floor( tileTex.y * tileSetVals.y ) );

    vec2 tileCoord = vec2( floor( pos.x * mapSizeInTiles.x ), floor( pos.y * mapSizeInTiles.y ) );

    // Get the tile's range in normalized coordinates
    vec2 tileNormX = vec2( tileCoord.x / mapSizeInTiles.x, ( tileCoord.x + 1 ) / mapSizeInTiles.x );
    vec2 tileNormY = vec2( tileCoord.y / mapSizeInTiles.y, ( tileCoord.y + 1 ) / mapSizeInTiles.y );

    // Check how far along the tile is this pos
    float tileRatioX = ( pos.x - tileNormX.x ) / ( tileNormX.y - tileNormX.x );
    float tileRatioY = ( pos.y - tileNormY.x ) / ( tileNormY.y - tileNormY.x );

    // Make sure the tile sprite doesn't bleed into the next
    if( tileRatioX == 1.0 ) tileRatioX = 0.999;
    if( tileRatioY == 1.0 ) tileRatioY = 0.999;

    vec4 tileColour = texture( tilesetTexture, vec2( tileSetVals.x * ( tileID.x + tileRatioX ) / tileSetVals.z,
        tileSetVals.x * ( tileID.y + tileRatioY ) / tileSetVals.z ) );
    fragColour = tileColour;
}
