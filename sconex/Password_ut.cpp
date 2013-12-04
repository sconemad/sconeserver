/* SconeServer (http://www.sconemad.com)

UNIT TESTS for Password

Copyright (c) 2000-2013 Andrew Wedgbury <wedge@sconemad.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program (see the file COPYING); if not, write to the
Free Software Foundation, Inc.,
59 Temple Place - Suite 330, Boston, MA  02111-1307, USA */

#include <sconex/Password.h>
#include <sconex/UnitTester.h>
using namespace scx;

bool test_rehash(PasswordHash* ph,
		 const std::string& password,
		 const std::string& hash,
		 int iter)
{
  for (int i=0; i<iter; ++i) {
    // There is obviously a small chance that this could fail if the rehash
    // happens to use the same random salt (but very unlikely).
    std::string newhash = ph->rehash(password);
    std::cout << newhash << "\n";
    if (newhash == hash) return false;
  }
  return true;
}

bool test_verify_wrong(PasswordHash* ph,
		       const std::string& hash)
{
  bool rehash = false;
  UTEST(!ph->verify("theWr0ngPa$$word",hash,rehash));
  UTEST(!ph->verify("",hash,rehash));
  UTEST(!ph->verify("aRea|!y$eCurePa55w@r?",hash,rehash));
  UTEST(!ph->verify("aRea|!y$eCUrePa55w@r?Z",hash,rehash));
  return true;
}
  
void Password_ut()
{
  UTSEC("hashing");
  UTCOD(std::string pw = "aRea|!y$eCUrePa55w@r?");

  UTMSG("md5_crypt");
  UTCOD(PasswordHash* ph_md5 = PasswordHash::create("md5_crypt",0));
  UTCOD(std::string hash_md5 = ph_md5->rehash(pw));
  std::cout << hash_md5 << "\n";
  UTEST(pw != hash_md5);
  UTEST(hash_md5.length() == 4 + 8 + 22);
  UTEST(test_rehash(ph_md5,pw,hash_md5,10));

  UTMSG("sha256_crypt");
  UTCOD(PasswordHash* ph_sha256 = PasswordHash::create("sha256_crypt",0));
  UTCOD(std::string hash_sha256 = ph_sha256->rehash(pw));
  std::cout << hash_sha256 << "\n";
  UTEST(pw != hash_sha256);
  UTEST(hash_sha256.length() == 4 + 16 + 43);
  UTEST(test_rehash(ph_sha256,pw,hash_sha256,10));

  UTMSG("sha512_crypt");
  UTCOD(PasswordHash* ph_sha512 = PasswordHash::create("sha512_crypt",0));
  UTCOD(std::string hash_sha512 = ph_sha512->rehash(pw));
  std::cout << hash_sha512 << "\n";
  UTEST(pw != hash_sha512);
  UTEST(hash_sha512.length() == 4 + 16 + 86);
  UTEST(test_rehash(ph_sha512,pw,hash_sha512,10));

  UTSEC("verification");

  bool rehash = false;

  UTMSG("md5_crypt");
  UTEST(ph_md5->verify(pw,hash_md5,rehash));
  UTEST(rehash == false);
  UTEST(test_verify_wrong(ph_md5,hash_md5));

  UTMSG("sha256_crypt");
  UTEST(ph_sha256->verify(pw,hash_sha256,rehash));
  UTEST(rehash == false);
  UTEST(test_verify_wrong(ph_sha256,hash_sha256));

  UTMSG("sha512_crypt");
  UTEST(ph_sha512->verify(pw,hash_sha512,rehash));
  UTEST(rehash == false);
  UTEST(test_verify_wrong(ph_sha512,hash_sha512));


  UTSEC("rehash notification");

  rehash = false;
  UTEST(ph_sha256->verify(pw,hash_md5,rehash));
  UTEST(rehash == true);

  rehash = false;
  UTEST(ph_sha512->verify(pw,hash_md5,rehash));
  UTEST(rehash == true);

  rehash = false;
  UTEST(ph_sha512->verify(pw,hash_sha256,rehash));
  UTEST(rehash == true);

}
