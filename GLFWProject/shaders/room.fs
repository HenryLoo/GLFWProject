#version 330 core

in vec2 texCoord;

out vec4 fragColour;

uniform sampler2D tilesetTexture;
uniform sampler2D layoutTexture;
uniform vec3 tileSetVals; // tileSize, numTilesInTilesetRow, textureSize
uniform vec2 mapSizeInTiles;

void main(){
    vec2 tileTex = texture(layoutTexture, texCoord).xy;

    // White pixel indicates an empty tile.
    if( tileTex.x == 1.0 && tileTex.y == 1.0 )
    {
        discard;
    }

    float tileSize = tileSetVals.x;
    float numTilesInTilesetRow = tileSetVals.y;
    float textureSize = tileSetVals.z;

    // Get the tile's id from the tileset.
    vec2 tileID = vec2(floor((tileTex.x / 1.0) * numTilesInTilesetRow),
        floor((tileTex.y / 1.0) * numTilesInTilesetRow));

    // OpenGL has (0, 0) at the bottom-left, but we want (0, 0) at the top-left.
    // numTilesInTilesetCol == numTilesInTilesetRow.
    tileID.y = numTilesInTilesetRow - 1 - tileID.y;

    // Get this fragment's tile coordinate in the room.
    vec2 tileCoord = vec2(floor(texCoord.x * mapSizeInTiles.x),
        floor(texCoord.y * mapSizeInTiles.y));

    // Get the tile's range in normalized coordinates
    vec2 normalizedTileSize = vec2(1 / mapSizeInTiles.x,
        1 / mapSizeInTiles.y);
    vec2 tileBottomLeft = vec2(tileCoord.x / mapSizeInTiles.x,
        tileCoord.y / mapSizeInTiles.y);

    // Get how far along the tile this fragment is at.
    vec2 tileRatio = vec2((texCoord.x - tileBottomLeft.x) / normalizedTileSize.x,
        (texCoord.y - tileBottomLeft.y) / normalizedTileSize.y);

    // Make sure the tile sprite doesn't bleed into the next.
    if( tileRatio.x >= 0.99 ) tileRatio.x = 0.99;
    if( tileRatio.y >= 0.99 ) tileRatio.y = 0.99;
    if( tileRatio.x <= 0.01 ) tileRatio.x = 0.01;
    if( tileRatio.y <= 0.01 ) tileRatio.y = 0.01;

    // Get this fragment's texture coordinate relative to its appropriate tile
    // on the tileset.
    vec2 tileTexCoord = vec2((tileID.x + tileRatio.x) * tileSize / textureSize,
         (tileID.y + tileRatio.y) * tileSize / textureSize);

    fragColour = texture(tilesetTexture, tileTexCoord);
}
