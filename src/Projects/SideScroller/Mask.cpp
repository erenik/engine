/// Emil Hedemalm
/// 2015-04-29
/// Mask

#include "Mask.h"
#include "File/File.h"
#include "String/StringUtil.h"

Mask::Mask()
	: name ("No name"), price(10000)
{
	Nullify();
}

Mask::Mask(String name, String textureSource, int price, int speedBonus /*= 0*/, Vector2i jumpBonus /* = Vector2i()*/)
	: name(name), textureSource(textureSource), price(price), speedBonus(speedBonus), jumpBonus(jumpBonus)
{
	Nullify();
}

void Mask::Nullify()
{
	purchased = false;	
}

Mask * Mask::GetByName(String name)
{
	for (int i = 0; i < masks.Size(); ++i)
	{
		Mask & mask = masks[i];
		if (mask.name == name)
			return &mask;
	}
	return NULL;
}

bool Mask::LoadFromCSV(String fileName)
{
	List<String> lines = File::GetLines(fileName);
	int nameCol = -1, priceCol = -1, speedBonusCol = -1, jumpBonusXCol = -1, jumpBonusYCol = -1, textureSourceCol = -1;
	for (int i = 0; i < lines.Size(); ++i)
	{
		String line = lines[i];
		List<String> elements = TokenizeCSV(line, ';');
		// First row.
		if (i == 0)
		{
			for (int j = 0; j < elements.Size(); ++j)
			{
				String element = elements[j];
				if (element == "Mask name")
					nameCol = j;
				else if (element == "Price")
					priceCol = j;
				else if (element == "Speed bonus")
					speedBonusCol = j;
				else if (element == "Jump bonus X")
					jumpBonusXCol = j;
				else if (element == "Jump bonus Y")
					jumpBonusYCol = j;
				else if (element == "Texture")
					textureSourceCol = j;
			}
			continue;
		}
		Mask newMask;
		for (int j = 0; j < elements.Size(); ++j)
		{
			String e = elements[j];
			if (j == nameCol)
				newMask.name = e;
			else if (j == priceCol)
				newMask.price = e.ParseInt();
			else if (j == speedBonusCol)
				newMask.speedBonus = e.ParseInt();
			else if (j == jumpBonusXCol)
				newMask.jumpBonus.x = e.ParseInt();
			else if (j == jumpBonusYCol)
				newMask.jumpBonus.y = e.ParseInt();
			else if (j == textureSourceCol)
			{
				if (!e.Contains("/"))
					e = "img/Masks/" + e;
				if (!e.Contains("."))
					e = e + ".png";
				newMask.textureSource = e;
			}
		}	
		if (newMask.name.Length() < 1)
			continue;
		// Save the mask!
		masks.AddItem(newMask);
	}
	return true;
}
