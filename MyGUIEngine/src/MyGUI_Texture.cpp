/*!
	@file
	@author		Albert Semenov
	@date		04/2009
	@module
*/
/*
	This file is part of MyGUI.
	
	MyGUI is free software: you can redistribute it and/or modify
	it under the terms of the GNU Lesser General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	
	MyGUI is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU Lesser General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public License
	along with MyGUI.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "MyGUI_Precompiled.h"
#include "MyGUI_Texture.h"
#include "OgreTextureManager.h"

namespace MyGUI
{

	Texture::Texture(const std::string& _name) :
		mName(_name)
	{
	}

	Texture::~Texture()
	{
		destroy();
	}

	void Texture::loadResource(Ogre::Resource* resource)
	{
		mLoader->loadResource(this);
	}

	const std::string& Texture::getName()
	{
		return mName;
	}

	void Texture::setManualResourceLoader(IManualResourceLoader* _loader)
	{
		mLoader = _loader;
	}

	void Texture::create()
	{
		mTexture = Ogre::TextureManager::getSingleton().create(
			mName,
			ResourceManager::getInstance().getResourceGroup(),
			true,
			mLoader == nullptr ? nullptr : this);
		mTexture->setTextureType(Ogre::TEX_TYPE_2D);
		mTexture->setNumMipmaps(0);
		mTexture->load();
	}

	void Texture::createManual(int _width, int _height, TextureUsage _usage, PixelFormat _format)
	{
		mTexture = Ogre::TextureManager::getSingleton().createManual(
			mName,
			ResourceManager::getInstance().getResourceGroup(),
			Ogre::TEX_TYPE_2D,
			_width,
			_height,
			0,
			getOgreFormat( _format ),
			getOgreUsage( _usage ),
			mLoader == nullptr ? nullptr : this);

		mTexture->load();
	}

	void Texture::loadFromMemory(const void* _buff, int _width, int _height, PixelFormat _format)
	{
		Ogre::PixelFormat format = getOgreFormat(_format);
		size_t data_size = _width * _height * getOgreNumByte(format);

		// FIXME �� ��� ������� ����, ������ ������� ������, � � ���� �� ���� ������ (�� ���� ���������)
#if OGRE_VERSION < MYGUI_DEFINE_VERSION(1, 6, 0)
		Ogre::DataStreamPtr memStream( new Ogre::MemoryDataStream(const_cast<void*>(_buff), data_size, true) );
#else
		Ogre::DataStreamPtr memStream( new Ogre::MemoryDataStream(const_cast<void*>(_buff), data_size) );
#endif

		Ogre::Image img;
		img.loadRawData(memStream, _width, _height, format);

		 //Call internal _loadImages, not loadImage since that's external and
		// will determine load status etc again, and this is a manual loader inside load()
		Ogre::ConstImagePtrList imagePtrs;
		imagePtrs.push_back(&img);

		mTexture->_loadImages( imagePtrs );
	}

	void Texture::loadFromFile(const std::string& _filename)
	{
		Ogre::TextureManager* manager = Ogre::TextureManager::getSingletonPtr();

		if ( false == manager->resourceExists(_filename) )
		{
			const std::string& group = ResourceManager::getInstance().getResourceGroup();
			if (!helper::isFileExist(_filename, group))
			{
				MYGUI_LOG(Error, "Texture '" + _filename + "' not found, set default texture");
			}
			else
			{
				mTexture = manager->load(_filename, group, Ogre::TEX_TYPE_2D, 0);
			}
		}
		else
		{
			mTexture = manager->getByName(_filename);
		}
	}

	void Texture::destroy()
	{
		Ogre::TextureManager::getSingleton().remove(mTexture->getName());
	}

	int Texture::getWidth()
	{
		return mTexture->getWidth();
	}

	int Texture::getHeight()
	{
		return mTexture->getHeight();
	}

	void* Texture::lock()
	{
		return mTexture->getBuffer()->lock(Ogre::HardwareBuffer::HBL_DISCARD);
	}

	void Texture::unlock()
	{
		mTexture->getBuffer()->unlock();
	}

	bool Texture::isLocked()
	{
		return mTexture->getBuffer()->isLocked();
	}

	PixelFormat Texture::getFormat()
	{
		Ogre::PixelFormat format = mTexture->getFormat();

		if (format == Ogre::PF_A8R8G8B8) return PixelFormat::A8R8G8B8;
		else if (format == Ogre::PF_BYTE_LA) return PixelFormat::L8A8;
		return PixelFormat::A8R8G8B8;
	}

	size_t Texture::getNumElemBytes()
	{
		return getOgreNumByte( mTexture->getFormat() );
	}

	Ogre::PixelFormat Texture::getOgreFormat(PixelFormat _format)
	{
		if (_format == PixelFormat::A8R8G8B8) return Ogre::PF_A8R8G8B8;
		else if (_format == PixelFormat::L8A8) return Ogre::PF_BYTE_LA;
		return Ogre::PF_A8R8G8B8;
	}

	size_t Texture::getOgreNumByte(Ogre::PixelFormat _format)
	{
		return Ogre::PixelUtil::getNumElemBytes(_format);
	}

	Ogre::TextureUsage Texture::getOgreUsage(TextureUsage _usage)
	{
		if (_usage == TextureUsage::Static) return Ogre::TU_STATIC;
		else if (_usage == TextureUsage::Dynamic) return Ogre::TU_DYNAMIC;
		else if (_usage == TextureUsage::WriteOnly) return Ogre::TU_WRITE_ONLY;
		else if (_usage == TextureUsage::StaticWriteOnly) return Ogre::TU_STATIC_WRITE_ONLY;
		else if (_usage == TextureUsage::DynamicWriteOnly) return Ogre::TU_DYNAMIC_WRITE_ONLY;
		else if (_usage == TextureUsage::DynamicWriteOnlyDiscardable) return Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE;
		else if (_usage == TextureUsage::RenderTarget) return Ogre::TU_RENDERTARGET;

		return Ogre::TU_DEFAULT;
	}

	TextureUsage Texture::getUsage(Ogre::TextureUsage _usage)
	{
		if (_usage == Ogre::TU_STATIC) return TextureUsage::Static;
		else if (_usage == Ogre::TU_DYNAMIC) return TextureUsage::Dynamic;
		else if (_usage == Ogre::TU_WRITE_ONLY) return TextureUsage::WriteOnly;
		else if (_usage == Ogre::TU_STATIC_WRITE_ONLY) return TextureUsage::StaticWriteOnly;
		else if (_usage == Ogre::TU_DYNAMIC_WRITE_ONLY) return TextureUsage::DynamicWriteOnly;
		else if (_usage == Ogre::TU_DYNAMIC_WRITE_ONLY_DISCARDABLE) return TextureUsage::DynamicWriteOnlyDiscardable;
		else if (_usage == Ogre::TU_RENDERTARGET) return TextureUsage::RenderTarget;

		return TextureUsage::Default;
	}

	TextureUsage Texture::getUsage()
	{
		return getUsage( (Ogre::TextureUsage)mTexture->getUsage() );
	}

} // namespace MyGUI
