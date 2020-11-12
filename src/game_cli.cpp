#define OLC_IMAGE_STB
#define OLC_PGE_APPLICATION
#include <cmath>
#include <vector>
#include <deque>
#include <string>
#include "olcPixelGameEngine/olcPixelGameEngine.h"

struct Character
{
	olc::vf2d pos; // The character's designated position
	olc::vf2d currPos; // The character's current position
	float theta; // Angle in radians from horizontal axis of line from current position to designated position
    std::string name; // Player name
    bool dancing; // Dancing flag
    double danceAngle; // Current angle of rotation in the dance

	Character()
	{
		pos = {0, 0};
		currPos = {0, 0};
		theta = 0;
        name = "Player";
        dancing = false;
        danceAngle = 0;
	}

	Character(olc::vf2d &p, olc::vf2d &cp, float t, std::string &n, bool d, double da)
	{
		pos = p;
		currPos = cp;
		theta = t;
        name = n;
        dancing = d;
        danceAngle = da;
	}

	void move(float x, float y) // Move the character to specified position and update theta
	{
		pos = {x, y};
		theta = atan2(currPos.y - pos.y, currPos.x - pos.x);
	}
};


class ClubBronco : public olc::PixelGameEngine
{
public:
	ClubBronco()
	{
		sAppName = "Club Bronco";
	}

private:
	Character player; // The player character
	float walkSpeed; // The walk speed of the player in pixels per second
    float danceSpeed; // The speed at which the player dances in radians per second
	std::unique_ptr<olc::Sprite> pAvatar; // The avatar of the player character
	std::unique_ptr<olc::Sprite> pAvatarFlip; // The flipped avatar of the player character
	std::unique_ptr<olc::Decal> dpAvatar; // Decal version of player avatar
	std::unique_ptr<olc::Decal> dpAvatarFlip; // Flipped decal of player avatar
	std::unique_ptr<olc::Sprite> bg; // The background image
	std::unique_ptr<olc::Decal> dbg; // Decal version of background image
	std::unique_ptr<olc::Sprite> arrow; // Arrow above the player
	std::unique_ptr<olc::Decal> darrow;
	std::unique_ptr<olc::Sprite> mbox; // Message box
	std::unique_ptr<olc::Decal> dmbox;
	std::unique_ptr<olc::Sprite> inputBox; // Input box
	std::unique_ptr<olc::Decal> dinputBox;
	std::unique_ptr<olc::Sprite> nameBox; // Name box
	std::unique_ptr<olc::Decal> dnameBox;
    olc::vf2d playerCenter; // Center of player sprite
	olc::vf2d origin; // The 0,0 position
	olc::vf2d arrowPos; // Position of arrow above player character
	olc::vf2d nameBoxPos; // Position of name box
	olc::vf2d mBoxPos; // Position of message box
	olc::vf2d inputBoxPos; // Position of input box
	float arrowSpace; // Space between bottom of arrow and top of player avatar
	olc::vf2d inputPos; // Position of input text
	olc::vf2d messagePos; // Position of message text
	olc::vf2d namePos; // Position of player names in name box
	static const int MAX_MESSAGES = 21; // Max number of displayed messages
	static const int MAX_INPUT_LENGTH = 40; // Max length of input string
	// For testing
	unsigned long long itr = 0;
public:
	// These variables will need to be updated by slave threads
	std::vector<Character> others; // List of other players
	std::deque<std::string> messages; // List of messages. Should limit to around 20
	std::string input; // The input string. Should limit to around 40 characters

	bool OnUserCreate() override
	{
		// Called once at the start, so create things here
		origin = {0, 0};
		player.pos = { float(ScreenWidth() / 2.0), float(ScreenHeight() / 2.0) };
		player.currPos = player.pos;
		float playerTheta = 0;
		walkSpeed = 100;
        danceSpeed = 1;
		pAvatar = std::make_unique<olc::Sprite>("./imgs/player.png");
		pAvatarFlip = std::make_unique<olc::Sprite>("./imgs/player_flip.png");
        playerCenter = {float(pAvatar->width / 2.0), float(pAvatar->height / 2.0)};
		dpAvatar = std::make_unique<olc::Decal>(pAvatar.get());
		dpAvatarFlip = std::make_unique<olc::Decal>(pAvatarFlip.get());
		bg = std::make_unique<olc::Sprite>("./imgs/bg.png");
		dbg = std::make_unique<olc::Decal>(bg.get());
		arrow = std::make_unique<olc::Sprite>("./imgs/arrow.png");
		darrow = std::make_unique<olc::Decal>(arrow.get());
		mbox = std::make_unique<olc::Sprite>("./imgs/message_box.png");
		dmbox = std::make_unique<olc::Decal>(mbox.get());
		inputBox = std::make_unique<olc::Sprite>("./imgs/input_box.png");
		dinputBox = std::make_unique<olc::Decal>(inputBox.get());
		nameBox = std::make_unique<olc::Sprite>("./imgs/name_box.png");
		dnameBox = std::make_unique<olc::Decal>(nameBox.get());
		arrowPos = {0, 0};
		arrowSpace = 5;
		mBoxPos = {20.0, float(ScreenHeight() - 290.0)};
		inputBoxPos = {20.0, float(ScreenHeight() - 50.0)};
		inputPos = {inputBoxPos.x + 10, float(inputBoxPos.y + inputBox->height / 2.0 - 3)};
		messagePos = {mBoxPos.x + 10, mBoxPos.y + 12};
		nameBoxPos = {20.0, mBoxPos.y - 24 - nameBox->height};
		namePos = {nameBoxPos.x + 12, nameBoxPos.y + 12};
		DrawSprite(origin, bg.get());
		// For testing
		input = "Test String";
		for (unsigned int i = 0; i < MAX_MESSAGES; ++i)
			messages.push_back("Test message");
		for (unsigned int i = 1; i <= 30; ++i)
		{
			others.push_back(player);
		}
		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// Called once per frame
		if (GetKey(olc::Key::ESCAPE).bPressed) // Quit with escape
			return false;
		if (GetMouse(0).bPressed) // Move player on mouse click
			player.move(GetMouseX() - (pAvatar->width / 2.0), GetMouseY() - (pAvatar->height / 2.0));
		// Gradually move the player towards the designated position
		if (sqrt(pow(player.currPos.x - player.pos.x, 2) + pow(player.currPos.y - player.pos.y, 2)) > walkSpeed * fElapsedTime)
		{
			// Because of the way the coordinate system works in the window, we have to subtract instead of add
			player.currPos.x -= walkSpeed * cos(player.theta) * fElapsedTime;
			player.currPos.y -= walkSpeed * sin(player.theta) * fElapsedTime;
		}
        // Handle dancing
        if (GetKey(olc::Key::DOWN).bPressed) // Toggle dancing for player
            player.dancing = !(player.dancing);
        if (player.dancing)
            player.danceAngle += danceSpeed * fElapsedTime;
        else
            player.danceAngle = 0;
		for (auto c : others) // Move and dance the other players
		{
			if (sqrt(pow(c.currPos.x - c.pos.x, 2) + pow(c.currPos.y - c.pos.y, 2)) > walkSpeed * fElapsedTime)
			{
				c.currPos.x -= walkSpeed * cos(c.theta) * fElapsedTime;
				c.currPos.y -= walkSpeed * sin(c.theta) * fElapsedTime;
			}
            if (c.dancing)
                c.danceAngle += danceSpeed * fElapsedTime;
            else
                c.danceAngle = 0;
		}
		arrowPos = {float(player.currPos.x + pAvatar->width / 2.0 - arrow->width / 2.0), float(player.currPos.y - arrow->height - arrowSpace)};

		for (auto c : others) // Draw the other players
        {
            if (-(walkSpeed * cos(c.theta)) < 0) // Determine if we need to flip the player
            {
                if (c.dancing) // Draw dancing player
                    DrawRotatedDecal(olc::vf2d(c.currPos.x + playerCenter.x, c.currPos.y + playerCenter.y), dpAvatarFlip.get(), sin(c.danceAngle) * (1.57 / 2), playerCenter, olc::vf2d(sin(c.danceAngle * 2) / 3 + 1, sin(c.danceAngle * 2) / 3 + 1));
                else
                    DrawDecal(c.currPos, dpAvatarFlip.get());
            }
            else
            {
                if (c.dancing)
                    DrawRotatedDecal(olc::vf2d(c.currPos.x + playerCenter.x, c.currPos.y + playerCenter.y), dpAvatar.get(), sin(c.danceAngle) * (1.57 / 2), playerCenter, olc::vf2d(sin(c.danceAngle * 2) / 3 + 1, sin(c.danceAngle * 2) / 3 + 1));
                else
                    DrawDecal(c.currPos, dpAvatar.get());
            }
        }
		// Draw the player
		if (-(walkSpeed * cos(player.theta)) < 0) // Determine if we need to flip the player
        {
            if (player.dancing) // Draw dancing player
                DrawRotatedDecal(olc::vf2d(player.currPos.x + playerCenter.x, player.currPos.y + playerCenter.y), dpAvatarFlip.get(), sin(player.danceAngle) * (1.57 / 2), playerCenter, olc::vf2d(sin(player.danceAngle * 2) / 3 + 1, sin(player.danceAngle * 2) / 3 + 1));
            else
                DrawDecal(player.currPos, dpAvatarFlip.get());
        }
		else
        {
            if (player.dancing)
                DrawRotatedDecal(olc::vf2d(player.currPos.x + playerCenter.x, player.currPos.y + playerCenter.y), dpAvatar.get(), sin(player.danceAngle) * (1.57 / 2), playerCenter, olc::vf2d(sin(player.danceAngle * 2) / 3 + 1, sin(player.danceAngle * 2) / 3 + 1));
            else
                DrawDecal(player.currPos, dpAvatar.get());
        }
		DrawDecal(arrowPos, darrow.get()); // Draw the arrow above player
		DrawDecal(mBoxPos, dmbox.get()); // Draw the message box
		DrawDecal(inputBoxPos, dinputBox.get()); // Draw the input box
		DrawStringDecal(inputPos, input, olc::WHITE); // Draw the input line
		DrawDecal(nameBoxPos, dnameBox.get()); // Draw the name box
		DrawStringDecal(namePos, player.name.substr(0, 12), olc::WHITE); // Draw the first 12 characters of player name in the name box
		// Draw messages
		for (unsigned int i = 0; i < messages.size(); ++i)
		{
			olc::vf2d drawPos = {messagePos.x, messagePos.y + i * 10};
			DrawStringDecal(drawPos, messages[i], olc::WHITE);
		}
		// Draw the other player names
		for (unsigned int i = 0; i < others.size(); ++i)
		{ // Should limit the max number of names drawn and the number of characters drawn later
			if (i > 28) // Only draw up to 29 other player names
				break;
			olc::vf2d drawPos = {namePos.x, namePos.y + (i + 1) * 12};
			DrawStringDecal(drawPos, others[i].name.substr(0, 12), olc::WHITE); // Only draw the first 12 characters of name
		}
		if (others.size() > 28) // If there are more than 28 other players in the room
		{
			// Indicate that there are undrawn names
			olc::vf2d drawPos = {namePos.x, namePos.y + 30 * 12};
			DrawStringDecal(drawPos, "...", olc::WHITE);
		}
		// For testing
		++itr;
		messages.pop_front();
		messages.push_back(std::to_string(itr));
		
		return true;
	}
};


int main()
{
	ClubBronco cb;
	if (cb.Construct(1280, 720, 1, 1))
		cb.Start();

	return 0;
}