/*
 *
 * $Id$
 *
 * This file is part of the KDE project, module kcontrol.
 * Copyright (C) 1999-2001 by Alex Zepeda
 *
 * You can freely distribute this program under the following terms:
 *
 * 1.) If you use this program in a product outside of KDE, you must 
 * credit the authors, and must mention "kcmemail".
 *
 * 2.) If this program is used in a product other than KDE, this license 
 * must be reproduced in its entirety in the documentation and/or some 
 * other highly visible location (e.g. startup splash screen)
 * 
 * 3.) Use of this product implies the following terms:
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#ifndef USERNAMEDLG_H
#define USERNAMEDLG_H "$Id"

#include <kdialogbase.h>

class KLineEdit;
class QLabel;
class QCheckBox;

class UserNameDlg
	: public KDialogBase
{
public:
	UserNameDlg (QWidget *parent, const QString &caption);
	~UserNameDlg ();

	QCheckBox *chkTLS;
	KLineEdit *txtUsername, *txtPass, *txtHost, *txtPort;
	QLabel *lblUsername, *lblPass, *lblHost, *lblPort;
};

#endif
