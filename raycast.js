const TILE_SIZE = 32;
const MAP_NUM_ROWS = 11;
const MAP_NUM_COLS = 15;

const WINDOW_WIDTH = MAP_NUM_COLS * TILE_SIZE;
const WINDOW_HEIGHT = MAP_NUM_ROWS * TILE_SIZE;

const FOV_ANGLE = 60 * (Math.PI / 180);

const WALL_STRIP_WIDTH = 30;
const NUM_RAYS = WINDOW_WIDTH / WALL_STRIP_WIDTH;

class Map {
    constructor() {
        this.grid = [ //15 * 11
            [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1],
            [1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1],
            [1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
            [1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1],
            [1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1],
            [1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1]
        ];
    }
    hasWallAt(x, y){
        if(x < 0 || x > WINDOW_WIDTH || y < 0 || y > WINDOW_HEIGHT)
        {
            return true;
        }
        var mapGridIndexX = Math.floor(x / TILE_SIZE);
        var mapGridIndexY = Math.floor(y / TILE_SIZE);
        return this.grid[mapGridIndexY][mapGridIndexX] == 1;
    }
    render() {
        for (var i = 0; i < MAP_NUM_ROWS; i++) {
            for (var j = 0; j < MAP_NUM_COLS; j++) {
                var tileX = j * TILE_SIZE; 
                var tileY = i * TILE_SIZE;
                var tileColor = this.grid[i][j] == 1 ? "#222" : "#fff";
                stroke("#222");
                fill(tileColor);
                rect(tileX, tileY, TILE_SIZE, TILE_SIZE);
            }
        }
    }
}

class Player {
    constructor() {
        this.x = WINDOW_WIDTH / 2;
        this.y = WINDOW_HEIGHT / 2;
        this.radius = 3; //描写毎の列
        this.turnDirection = 0;
        this.walkDirection = 0;
        this.rotationAngle = Math.PI / 2;
        this.moveSpeed = 2.0;
        this.rotationSpeed = 2 * (Math.PI / 180)
    }
    update(){
        this.rotationAngle += this.turnDirection * this.rotationSpeed;
        var moveStep = this.walkDirection * this.moveSpeed;
        // this.x += Math.cos(this.rotationAngle) * moveStep;
        // this.y += Math.sin(this.rotationAngle) * moveStep;
        var newPlayerX = this.x + Math.cos(this.rotationAngle) * moveStep;
        var newPlayerY = this.y + Math.sin(this.rotationAngle) * moveStep;
        if (!grid.hasWallAt(newPlayerX, newPlayerY)){
            this.x = newPlayerX;
            this.y = newPlayerY;
        }
    };
    render(){
        noStroke();
        fill("red");
        circle(this.x, this.y, this.radius);
        stroke("red");
        line(this.x, this.y, this. x + Math.cos(this.rotationAngle) * 30, this.y + Math.sin(this.rotationAngle) * 30)
    }
}

class Ray {
    constructor(rayAngle){
        this.rayAngle = rayAngle;
    }
    render(){
        stroke("red")
        line(
            player.x,
            player.y,
            player.x + Math.cos(this.rayAngle) * 30,
            player.y + Math.sin(this.rayAngle) * 30
        )
    }
}

var grid = new Map();
var player = new Player();
var rays = [];

function keyPressed(){
    if (keyCode == UP_ARROW){
        player.walkDirection += 1;
    } else if(keyCode == DOWN_ARROW){
        player.walkDirection -= 1;
    } else if(keyCode == RIGHT_ARROW){
        player.turnDirection += 1;
    } else if(keyCode == LEFT_ARROW){
        player.turnDirection -= 1;
    }
}

function keyReleased(){
    if (keyCode == UP_ARROW){
        player.walkDirection = 0;
    } else if(keyCode == DOWN_ARROW){
        player.walkDirection = 0;
    } else if(keyCode == RIGHT_ARROW){
        player.turnDirection = 0;
    } else if(keyCode == LEFT_ARROW){
        player.turnDirection = 0;
    }
}

function castAllRays() {
    var columnId = 0;
    var rayAngle = player.rotationAngle - (FOV_ANGLE / 2)

    rays = [];

    for (var i = 0; i < NUM_RAYS; i++)
    {
        var ray = new Ray(rayAngle)
        rays.push(ray)
        rayAngle +=  FOV_ANGLE / NUM_RAYS;
        columnId++;
    }
}

function setup() {
    createCanvas(WINDOW_WIDTH, WINDOW_HEIGHT);
}

function update() {
    player.update();
    castAllRays();
    // TODO: update all game objects before we render the next frame
}

function draw() {
    update();
    grid.render();
    for (ray of rays){
        ray.render();
    }
    player.render();
}
